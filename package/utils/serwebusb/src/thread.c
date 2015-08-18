/*
 * thread.c
 *
 * Multithreading routines
 *
 * (C)1999 Stefano Busti
 *
 */

#include "thread.h"

int thr_create(thr_t *thread, int detached,
			   thr_startfunc_ptr_t startfunc, void *arg)
{
#if defined(PTHREADS)
	int res;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, detached
								? PTHREAD_CREATE_DETACHED
								: PTHREAD_CREATE_JOINABLE);
								
	res = pthread_create(thread, &attr, startfunc, arg);
	pthread_attr_destroy(&attr);
	
	return res;
		
#elif defined(WINTHREADS)
	DWORD ThreadID;
	HANDLE hThread = CreateThread(NULL, 0, startfunc, arg, 0, &ThreadID);

	if (hThread == NULL)
		return GetLastError();

	*thread = hThread;

	return 0;
#endif
}

int thr_cancel(thr_t thread)
{
#if defined(PTHREADS)
	return pthread_cancel(thread);
#elif defined(WINTHREADS)
	BOOL res = TerminateThread(thread, 0);
	if (!res)
		return GetLastError();
	return 0;
#endif
}

void thr_exit(thr_exitcode_t retval)
{
#if defined(PTHREADS)
	pthread_exit(retval);
#elif defined(WINTHREADS)
	ExitThread(retval);
#endif
}

thr_t thr_self(void)
{
#if defined(PTHREADS)
	return pthread_self();
#elif defined(WINTHREADS)
	return GetCurrentThread();
#endif
}

int thr_equal(thr_t thread1, thr_t thread2)
{
#if defined(PTHREADS)
	return pthread_equal(thread1, thread2);
#elif defined(WINTHREADS)
	return thread1 == thread2;
#endif
}

int thr_join(thr_t thread, thr_exitcode_t *retval)
{
#if defined(PTHREADS)
	return pthread_join(thread, retval);
#elif defined(WINTHREADS)
	return -1;
#endif
}

int thr_detach(thr_t thread)
{
#if defined(PTHREADS)
	return pthread_detach(thread);
#elif defined(WINTHREADS)
	return 0;
#endif
}

/*
 * Mutex functions
 */

thr_mutex_t thr_mutex_create(void)
{
#if defined(PTHREADS)
	thr_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

	return mutex;
#elif defined(WINTHREADS)
	return CreateMutex(NULL, FALSE, NULL);
#endif
}

int thr_mutex_destroy(thr_mutex_t *mutex)
{
#if defined(PTHREADS)
	return pthread_mutex_destroy(mutex);
#elif defined(WINTHREADS)
	return CloseHandle(*mutex) ? 0 : -1;
#endif
}

int thr_mutex_lock(thr_mutex_t *mutex)
{
#if defined(PTHREADS)
	return pthread_mutex_lock(mutex);
#elif defined(WINTHREADS)
	return WaitForSingleObject(*mutex, INFINITE) == WAIT_TIMEOUT ? -1 : 0;
#endif
}

int thr_mutex_trylock(thr_mutex_t *mutex)
{
#if defined(PTHREADS)	
	return pthread_mutex_trylock(mutex);
#elif defined(WINTHREADS)
	return WaitForSingleObject(*mutex, 0) == WAIT_TIMEOUT ? -1 : 0;
#endif
}

int thr_mutex_unlock(thr_mutex_t *mutex)
{
#if defined(PTHREADS)
	return pthread_mutex_unlock(mutex);
#elif defined(WINTHREADS)
	return ReleaseMutex(*mutex) ? 0 : -1;
#endif
}
