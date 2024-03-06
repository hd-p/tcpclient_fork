#ifndef __PTHREADS_H__
#define __PTHREADS_H__


#include <pthread.h>

typedef enum PTHREAD_CANCEL_TYPE{
	PTHREAD_CANCEL_DEF,
	PTHREAD_CANCEL_TEST_DORP,
	PTHREAD_CANCEL_RIGHT_AWAY,
	PTHREAD_CANCEL_NONE,
	
};


int PThreadCancel(pthread_t ptId, void **retval);
int PThreadCancelSW(int sw);
int InitTcpRecvPthread(void);

#endif
