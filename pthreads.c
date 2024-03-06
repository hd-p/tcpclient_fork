#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "tcpclient.h"
#include "pthreads.h"



pthread_t pthreadTcpC;

int PThreadCancel(pthread_t ptId, void **retval)
{
	if(ptId < 0)
	{
		printf("%s: unable ptherad.\r\n");
		return -1;
	}
	
	pthread_cancel(ptId);
	pthread_join(ptId, retval);
	
	return 0;
}

int PThreadCancelSW(int sw)
{
	if(sw < 0)
	{
		printf("%s: unable input(EN or DIS).\r\n");
		return -1;
	}
	
	switch (sw)
	{
		case PTHREAD_CANCEL_DEF:
		case PTHREAD_CANCEL_TEST_DORP:
		{
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
			pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
			break;
		}
		case PTHREAD_CANCEL_RIGHT_AWAY:
		{
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
			pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
			break;
		}
		case PTHREAD_CANCEL_NONE:
		{
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			
			break;
		}
		
	}
}




void *pthreadFuncTcpCRcv(void)
{
	fd_set rfds;
	int iInx = 0, iRMaxFd, iRet = 0;
	struct timeval tmval;
	char szBuf[10 * 1024] = {0};
	
	PThreadCancelSW(PTHREAD_CANCEL_DEF);
	while(1)
	{
		printf("123\r\n");	
		
		FD_ZERO(&rfds);
		
		if(giTcpSock > 0)
		{
			FD_SET(giTcpSock, &rfds);
			iRMaxFd = giTcpSock + 1;
		}
		else
		{
			sleep(1);
			continue;
		}
			
			
		tmval.tv_sec = 2;
		tmval.tv_usec = 0;
		
		pthread_testcancel();
		
		iRet = select(iRMaxFd, &rfds, NULL, NULL, &tmval);
		pthread_testcancel();
		switch(iRet)
		{
			case -1:
				printf("%s: select err, %d(%s)", __FUNCTION__, errno, strerror(errno));
				if(errno == EINTR)
					continue;
				
				goto failerr;
			case 0:
				continue;
			
			default:
				if(FD_ISSET(giTcpSock, &rfds))
				{
					TcpRecv(giTcpSock, szBuf, sizeof(szBuf));
				}
				break;
		}
			
		
	}
	
failerr:
	CloseFd(giTcpSock);
	return -1;
}

int InitTcpRecvPthread(void)
{
	int iRet = 0;
	
	iRet = pthread_create(&pthreadTcpC, NULL, pthreadFuncTcpCRcv, NULL);
	if(iRet < 0)
	{
		printf("tcp client open fail.\r\n");
		return -1;
	}
	pthread_detach(pthreadTcpC);
	return 0;
}

