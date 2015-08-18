#ifndef THREAD_H
#define THREAD_H

#if defined(__UNIX__)
#  define PTHREADS
#elif defined(__WIN32__)
#  define WINTHREADS
#else
#  define WINTHREADS
#endif

#if defined(PTHREADS)

#  include <pthread.h>
typedef pthread_t thr_t;
typedef pthread_mutex_t thr_mutex_t;
typedef void * thr_exitcode_t;
typedef void *(*thr_startfunc_ptr_t)(void *);
typedef void * thr_startfunc_t;

#elif defined(WINTHREADS)

#  include <windows.h>
typedef HANDLE thr_t;
typedef HANDLE thr_mutex_t;
typedef DWORD thr_exitcode_t;
typedef LPTHREAD_START_ROUTINE thr_startfunc_ptr_t;
#define thr_startfunc_t thr_exitcode_t WINAPI
#endif

int thr_create(thr_t *thread, int detached,
			   thr_startfunc_ptr_t startfunc, void *arg);
int thr_cancel(thr_t thread);
void thr_exit(thr_exitcode_t retval);
thr_t thr_self(void);
int thr_equal(thr_t thread1, thr_t thread2);
int thr_join(thr_t thread, thr_exitcode_t *retval);
int thr_detach(thr_t thread);

thr_mutex_t thr_mutex_create(void);
int thr_mutex_destroy(thr_mutex_t *mutex);
int thr_mutex_lock(thr_mutex_t *mutex);
int thr_mutex_trylock(thr_mutex_t *mutex);
int thr_mutex_unlock(thr_mutex_t *mutex);

#endif	/* THREAD_H */
