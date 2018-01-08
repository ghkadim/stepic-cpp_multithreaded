#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define WAIT_FOR_LOCK_FUNCTION(FUNC_NAME, LOCK_TYPE, LOCK_FUNC, UNLOCK_FUNC) \
void* FUNC_NAME(void* arg) { \
    LOCK_TYPE *lock = (LOCK_TYPE *) arg; \
    if(LOCK_FUNC(lock) != 0) { \
        perror( #LOCK_FUNC ); \
        return (void*)EXIT_FAILURE; \
    } \
    if(UNLOCK_FUNC(lock) != 0) { \
        perror( #UNLOCK_FUNC ); \
        return (void*)EXIT_FAILURE; \
    } \
    return (void*)EXIT_SUCCESS; \
}

WAIT_FOR_LOCK_FUNCTION( wait_for_mutex, pthread_mutex_t, pthread_mutex_lock, pthread_mutex_unlock )
WAIT_FOR_LOCK_FUNCTION( wait_for_spinlock, pthread_spinlock_t, pthread_spin_lock, pthread_spin_unlock )
WAIT_FOR_LOCK_FUNCTION( wait_for_rdlock, pthread_rwlock_t, pthread_rwlock_rdlock, pthread_rwlock_unlock )
WAIT_FOR_LOCK_FUNCTION( wait_for_wrlock, pthread_rwlock_t, pthread_rwlock_wrlock, pthread_rwlock_unlock )

#define RETURN_ERROR(MSG) \
do { \
    perror(MSG); \
    return EXIT_FAILURE; \
} while(0)

#define INIT_AND_LOCK(LOCK, INIT_FUNC, ARG, LOCK_FUNC) \
do { \
    if(INIT_FUNC(LOCK, ARG) != 0) \
        RETURN_ERROR(#INIT_FUNC); \
    if(LOCK_FUNC(LOCK) !=0) \
        RETURN_ERROR(#LOCK_FUNC); \
} while(0)

typedef void* (*thread_func)(void*);
typedef void* thread_arg;

int main() {
    pthread_spinlock_t spinlock;
    pthread_mutex_t mutex;
    pthread_rwlock_t rwlock;
    void* res;

    pthread_t threads[4];
    thread_func funcs[4] = {wait_for_mutex, wait_for_spinlock, wait_for_rdlock, wait_for_wrlock};
    thread_arg args[4] = {&mutex, &spinlock, &rwlock, &rwlock};

    INIT_AND_LOCK(&mutex, pthread_mutex_init, NULL, pthread_mutex_lock);
    INIT_AND_LOCK(&spinlock, pthread_spin_init, 0, pthread_spin_lock);
    INIT_AND_LOCK(&rwlock, pthread_rwlock_init, NULL, pthread_rwlock_wrlock);

    for(int i=0; i<4; i++) {
        if(pthread_create(&(threads[i]), NULL, funcs[i], args[i]) != 0)
            RETURN_ERROR("pthread_create");
    }

    for(int i=0; i<4; i++) {
        if(pthread_join(threads[i], &res) != 0)
            RETURN_ERROR("pthread_join");
    }

    return EXIT_SUCCESS;
}
