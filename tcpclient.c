#include <stdio.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
// #include <sys/type.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h> // SOL_TCP
#include <netdb.h>

int giTcpSock = 0;


int TcpRecv(int iFd, char *pBuf, int iLen);

void CloseFd(int iFd)
{
	if(iFd > 0)
	{
		close(iFd);
		iFd = -1;
	}
	
}


static int connect_intime(int socket_fd,struct sockaddr_in host,int iTime)
{
	struct timeval timeout={3,0};
	fd_set mask;
	int	err; 
	int errlen;	
	int status;
	
	status=connect(socket_fd,(struct sockaddr *)&host,sizeof(host)); 
	if(-1==status)
	{
		if(EINPROGRESS!=errno)
		{//不是正在连接中
			printf("\nsockfd: %d connect failed!\n", socket_fd);
			return 0;
		}
	}
	timeout.tv_sec=iTime; 
	timeout.tv_usec=0; 		/* timeout value*/ 
	FD_ZERO(&mask);
	FD_SET(socket_fd,&mask);
	printf("\nwait %ds to connect.\n",iTime);
	status = select(socket_fd+1,(fd_set *)0, &mask, (fd_set *)0, &timeout);
	switch(status)
	{
		case -1:
		{/* select erro */
			printf("\nsockfd: %d select error!\n", socket_fd + 1);
			return 0;
		}
		case 0:
		{//time out					
			printf("\nsockfd: %d connect timeout!\n", socket_fd);
			return 0;
		}
		default: 	
		{//success
			printf("\nsockfd: %d select ret!\n", socket_fd);
			if( FD_ISSET(socket_fd,&mask) ) 
			{
				err=1; 
				errlen=1;
				//主要针对防火墙
				getsockopt(socket_fd,SOL_SOCKET,SO_ERROR,(char*)&err,(socklen_t *)&errlen); 
				if(err==0)
				{	
					printf("\nconnect to dsc sus.\n");					
					return 1;
				}
				else
				{
					printf("\nconnect to dsc fail.because fialwall,err:%d(%s).\n", err, strerror(err));		
					return 0;
				}
			}
			printf("\nsockfd: %d select err.not socket fd!\n", socket_fd);
			break;
		}
	}
	return 0;
}

int InitTCPClient(char *pIp, int iPost)
{
	if(pIp == NULL || iPost <= 0)
	{
		printf("%s: input fail.\r\n");
		return -1;
	}
	
	printf("%s: link %s:%d\r\n", __FUNCTION__, pIp, iPost);
	
	int iInx = 0, optlen = 0, sockfd = -1, rsflags = 0;
	char szBuf[64];
	struct sockaddr_in sock_in;
	struct hostent* pht = NULL;
	
	memset(szBuf, 0, sizeof(szBuf));
	memset((char *)&sock_in, 0, sizeof(struct sockaddr_in));
	
	memcpy(szBuf, pIp, strlen(pIp));
	
	sock_in.sin_family = AF_INET;
	sock_in.sin_port = htons(iPost);
	
	if(inet_aton(szBuf, &sock_in.sin_addr))
	{
		;
	}
	else
	{
		/* dns get ip */
		pht = gethostbyname(szBuf);
		if (!pht)
		{
			printf("\n%s: unknown host %s\n",__FUNCTION__,szBuf);            
			goto tcp_conn_error_out;
		}
		
		if (pht->h_length > (int)sizeof(struct in_addr)) 
		{
			pht->h_length = sizeof(struct in_addr);
		}
		memcpy(&sock_in.sin_addr, pht->h_addr, pht->h_length);
		printf("\ngot ip from domain:%s(%s)\n",szBuf,inet_ntoa(*(struct in_addr *)&sock_in.sin_addr.s_addr));		
	}
	
	if ( (sockfd=socket(AF_INET,SOCK_STREAM,0))<0 ) 
	{ 
		printf("\n%s: create sock err,errno:%d(%s).",__FUNCTION__,errno,strerror(errno));
		goto tcp_conn_error_out; 
	}
	// rsflags = fcntl(sockfd,F_GETFL);
	// rsflags |= O_NONBLOCK;
	// fcntl(sockfd, F_SETFL, rsflags);	/* set socket to NONBLOCK */
	
#define SET_SOCKET_OPT // 心跳包（保活包）
#ifdef SET_SOCKET_OPT
{
	int keepAlive 	= 1; 	// 开启keepalive属性
	int keepIdle 		= 60; 	// 如该连接在60秒内没有任何数据往来,则进行探测 
	int keepInterval 	= 5; 	// 探测时发包的时间间隔为5 秒
	int keepCount 	= 8; 	// 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.

	if(setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive))<0)
	{
		
		printf("SetSockopt err.error:%d(%s)",errno,strerror(errno));
	}
	if(setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle))<0)
	{
		
		printf("SetSockopt err.error:%d(%s)",errno,strerror(errno));
	}
	if(setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval))<0)
	{
		
		printf("SetSockopt err.error:%d(%s)",errno,strerror(errno));
	}
	if(setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount))<0)
	{
		
		printf("SetSockopt err.error:%d(%s)",errno,strerror(errno));
	}
}
#endif
	if(!connect_intime(sockfd,sock_in,20))
	{
    	    printf("\nconnect dsc fail");
    	    goto tcp_conn_error_out;
	}
	
	size_t snd_size = 64*1024;    /* 发送缓冲区大小为8K */ 
	optlen = sizeof(snd_size); 
	int err = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &snd_size, optlen); 
	if(err<0)
	{ 
		printf("Set Buff Err!!!\n"); 
	} 

	return sockfd;
tcp_conn_error_out:
	if(sockfd>=0)
		close(sockfd);
	return -1;
	
	
}

int TcpClientSend(int iFd, char *pBuf, int iLen)
{
	if(iFd < 0 || pBuf == NULL || iLen < 0)
	{
		printf("%s: input fail(%d).\r\n", __FUNCTION__, iFd);
		return -1;
	}
	
	int iRet = 0;
	static iInx = 0;
	
	iRet = send(iFd, pBuf, iLen, 0);
	if(iRet != iLen)
	{
		printf("%s: tcp send fail\r\n", __FUNCTION__);
		return -1;
	}
	printf("%s: index:%d, %s\r\n", __FUNCTION__, (++iInx)*iLen, pBuf);
	
}




int TcpRecv(int iFd, char *pBuf, int iLen)
{
	if(iFd < 0 || pBuf == NULL || iLen < 0)
	{
		printf("%s: input fail.\r\n", __FUNCTION__);
		return -1;
	}
	
	int iRet = 0;
	char *pRcvBuf;
	static int iInx = 0;
	
	// pRcvBuf = (char *)malloc(iLen * sizeof(char));
	pRcvBuf = (char *)calloc(iLen, sizeof(char));
	
	
	iRet = recv(iFd, pRcvBuf, iLen, 0);
	switch(iRet)
	{
		case -1:
			printf("%s: socket recv -1.\r\n", __FUNCTION__);
			goto failclose;
			break;
			
		case 0:
			printf("%s: socket recv 0.\r\n", __FUNCTION__);
			goto failclose;
			break;
		
		default:
			memcpy(pBuf, pRcvBuf, iLen);
			iInx += iRet;
			printf("%s: %d:%d >>>>> %s\r\n", __FUNCTION__, iInx, iRet, pRcvBuf);
			free(pRcvBuf);
			return iRet;
	}
	
failclose:
	CloseFd(iFd);
	free(pRcvBuf);
	return -1;

		
	
}

int TcpTest(void)
{
	
	struct in_addr addr;
	char szBuf[64] = {0};
	char *pBuf = NULL;
	
	addr.s_addr = htonl(INADDR_ANY);
	pBuf = inet_ntoa(addr);
	
	printf("local ip %s\r\n", pBuf);
	// addr = inet_aton()
}


int GetIpPost(char *pBuf, char *pIp, int *piPost)
{
	if(pBuf == NULL || pIp == NULL || piPost == NULL)
	{
		printf("%s: input fail.\r\n", __FUNCTION__);
		return -1;
	}
	
	char szBuf[32];
	int iPost = 0;
	
	// sscanf("192.168.23.37:8000", "%[^:]", szBuf);
	if(sscanf(pBuf, "%[^:]:%d", szBuf, &iPost))
	{
		printf("%s: %s : %d.\r\n", __FUNCTION__, szBuf, iPost);
		memcpy(pIp, szBuf, strlen(szBuf));
		*piPost = iPost;
	}
	else
		printf("%s: sscanf fail.\r\n", __FUNCTION__);
		
	return 0;
}


