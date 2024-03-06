#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "tcpclient.h"
#include "pthreads.h"
#include "fork_process.h"


char *MmapSockFd(int iFd)
{
	if(iFd <= 0)
	{
		printf("input fail.\r\n");
		return NULL;
	}

	int iInx = 0;
	void * m_pBuf;
	char *pBuf = NULL;
	int err = 0;

	unsigned long ulLen = 0;
	socklen_t Lensize;

	getsockopt(iFd, SOL_SOCKET, SO_SNDBUF, &ulLen, &Lensize);
	if(err<0)
	{ 
		printf("Set Buff Err!!!\n"); 
		return NULL;
	} 

	m_pBuf = mmap(NULL, ulLen, PROT_READ | PROT_WRITE,
		 MAP_SHARED, iFd,0);

	if(m_pBuf == NULL)
	{
		printf("mmap buffer fail.\r\n");
		return NULL;
	}

	pBuf = (char *)m_pBuf;
	return pBuf;
	
}


int main(int argc, void *argv[])
{
	if(argc != 2)
		return 0;
	int iInx = 0;
	
	char szIp[32] = {0};
	int iPost = 0;
	char *pBuf = NULL;
	
	InitTcpRecvPthread();
	GetIpPost(argv[1], szIp, &iPost);
	giTcpSock = InitTCPClient(szIp, iPost);

	// pBuf = MmapSockFd(giTcpSock);
	iInx = ForkProcess(1);
	while(1)
	{
		// printf("456\r\n");
		if(gpid == 0)
		{
			printf("=========%d=========", iInx);
			TcpClientSend(giTcpSock, "456aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", strlen("456aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
			usleep(100 * 1000);
		}
		
	}
	
}


