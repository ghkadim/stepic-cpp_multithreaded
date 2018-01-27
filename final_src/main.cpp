#include <iostream>
#include <fstream>
#include <memory>
#include <utility>
#include <vector>
#include <thread>
#include <type_traits>
#include <boost/asio.hpp>

namespace ba = boost::asio;
namespace bs = boost::system;
using ba::ip::tcp;

using namespace std;

#define LOG_ON

#ifdef LOG_ON
#define LOG(...) \
do { \
    std::cout << "[" << hex << std::this_thread::get_id() << "] " \
              << __VA_ARGS__ << std::endl; \
} while(false)
#else
#define LOG(...) do {} while(false)
#endif

string directory;

const string FILE_NOT_FOUND =
R"(<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head>
   <title>404 Not Found</title>
</head>
<body>
   <h1>Not Found</h1>
   <p>The requested URL was not found on this server.</p>
</body>
</html>)";

class http_request {
public:
    http_request() {}
    http_request(string url)
        : is_full_{true},
          url_{url}
    {}

    string get_url() const { return url_; }

    bool is_full() const {
        return is_full_;
    }
private:
    bool is_full_{false};
    string url_;
};

class http_response {
public:

    enum class code {
        OK = 200,
        NOT_FOUND = 404
    };

    void set_code(code c) {
        data_buf_ = "HTTP/1.1 ";
        switch(c) {
            case(code::OK):
                data_buf_ += "200 OK\r\n";
                break;
            case(code::NOT_FOUND):
            default:
                data_buf_ += "200 Not Found\r\n";
        }
    }

    void add_header(const string& name, const string& value) {
        data_buf_ += name;
        data_buf_ += ": ";
        data_buf_ += value;
        data_buf_ += "\r\n";
    }

    void set_data(const string& data) {
        add_header("Content-Length", to_string(data.size() + 2));
        data_buf_ += "\r\n\r\n";
        data_buf_ += data;

        LOG("Response: \n" << data_buf_);
    }

    void set_data_file(const string& file_path) {
        add_header("Content-Type", "text/html");

        ifstream infile(file_path, std::ios::ate);
        size_t file_size = infile.tellg();

        string file_data(file_size, '\0');
        infile.seekg(0);

        if(infile.read(&file_data[0], file_size))
            set_data(file_data);
        else
            throw runtime_error("Failed to read file");
    }

    using buffer = decltype(ba::buffer(declval<const char*>(), declval<size_t>()));

    buffer get_buffer() const {
        return ba::buffer(data_buf_.c_str(), data_buf_.size());
    }

private:
    string data_buf_;
};

class http_handler
{
public:
    void do_get(const http_request& request, http_response& response) {
        string file_path = directory + request.get_url();
        if(file_exists(file_path)) {
            LOG("Send content of file: " << file_path);
            response.set_code(http_response::code::OK);
            response.set_data_file(file_path);
        } else {
            LOG("File not found: " << file_path);
            response.set_code(http_response::code::NOT_FOUND);
            response.set_data(FILE_NOT_FOUND);
        }
    }

private:
    bool file_exists(const string& path) {
        ifstream infile(path);
        return infile.good();
    }
};

class session
    : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket)
        : socket_(std::move(socket))
    {}

    void start()
    {
        do_read();
    }

private:
    void do_read()
    {
        LOG("do_read");

        auto self(shared_from_this());
        socket_.async_read_some(ba::buffer(data_chunk_, chunk_size),
                [this, self](bs::error_code ec, std::size_t length)
        {
            if(ec) {
                LOG("Read error: " << ec.message());
                return;
            }

            LOG("Data received");

            data_ += string(data_chunk_, length);
            http_request request = compose_request();

            if(request.is_full()) {
                LOG("Request: " << endl << data_);

                try {
                    http_handler handler;
                    handler.do_get(request, response_);
                } catch (exception& ex) {
                    LOG("Failed to proceed request: " << ex.what());
                    response_.set_code(http_response::code::NOT_FOUND);
                    response_.set_data(FILE_NOT_FOUND);
                }

                do_write();
            } else {
                do_read();
            }
        });
    }

    void do_write()
    {
        LOG("do_write");

        auto self(shared_from_this());
        ba::async_write(socket_, response_.get_buffer(),
            [this, self](bs::error_code ec, std::size_t /*length*/)
        {
            if(ec) {
                LOG("Failed to write: " << ec.message());
            }
            do_wait();
        });
    }

    void do_wait()
    {
        LOG("do_wait");

        auto self(shared_from_this());
        socket_.async_read_some(ba::buffer(data_chunk_, chunk_size),
                [this, self](bs::error_code ec, std::size_t length)
        {
            if(ec) {
                LOG("Waiting done: " << ec.message());
            }
        });
    }

    http_request compose_request() {
        if(data_.find("\n") == string::npos) {
            return http_request{};
        }

        size_t pos_arr[3] = {0};
        for(int i=1; i<3; i++) {
            pos_arr[i] = data_.find(" ", pos_arr[i-1] + 1);
        }

        if(pos_arr[1] == string::npos || pos_arr[2] == string::npos) {
            return http_request{};
        }

        return http_request{data_.substr(pos_arr[1] + 1, pos_arr[2] - pos_arr[1] - 1)};
    }

    tcp::socket socket_;

    static const size_t chunk_size=1024;
    char data_chunk_[chunk_size];
    string data_;

    http_response response_;
};

class server
{
public:
    server(ba::io_service& io_service,
           std::string addr,
           unsigned short port,
           std::string dir)
        : acceptor_(io_service,
                    tcp::endpoint(ba::ip::address::from_string(addr), port)),
          socket_(io_service)
    {
        directory = dir;
        LOG("Server started");
        do_accept();
    }

private:
  void do_accept()
  {
    acceptor_.async_accept(socket_,
        [this](bs::error_code ec)
        {
          if (!ec)
          {
            LOG("Accepted new client");
            std::make_shared<session>(std::move(socket_))->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  tcp::socket socket_;
};

/*
 * http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html
 */
void daemonize() {
    pid_t pid, sid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);

    /* Open any logs here */

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        /* Log any failures here */
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        /* Log any failures here */
        exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main(int argc, char* argv[])
{
    try {
        int opt;

        string ip;
        unsigned short port;
        string dir;
        bool goto_daemon{true};

        while((opt = getopt(argc, argv, "h:p:d:n")) != -1) {
            switch (opt) {
                case 'h':
                    ip = string{optarg};
                    break;
                case 'p':
                    port = atoi(optarg);
                    break;
                case 'd':
                    dir = string{optarg};
                    break;
                case 'n':
                    goto_daemon = false;
                    break;
                default: /* '?' */
                    fprintf(stderr, "Usage: %s [-h ip] [-p port] [-d directory] [-n]\n",
                            argv[0]);
                    exit(EXIT_FAILURE);
            }
        }
        LOG("ip:" << ip);
        LOG("port:" << port);
        LOG("directory:" << dir);

        if(goto_daemon) {
            daemonize();
            LOG("daemonized");
        }

        ba::io_service io_service;

        server s(io_service, ip, port, directory);

        const int thread_count = 4;
        using thread_list = vector<thread>;
        thread_list threads;

        unique_ptr<thread_list, function<void(thread_list*)>>
            threads_join{&threads, [](thread_list* threads)
        {
            for(auto& t: *threads) {
                t.join();
            }
        }};

        for(int i=0; i<thread_count; ++i) {
            threads.emplace_back([&]() {
                LOG("Created thread");
                io_service.run();
            });
        }
    }
    catch (std::exception& e) {
        LOG("Exception: " << e.what());
    }
    catch(...) {
        LOG("Unknown exception");
    }

    LOG("Bye!");

    return 0;
}
