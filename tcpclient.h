#ifndef __TCPCLIENT_H__
#define __TCPCLIENT_H__



extern int giTcpSock;


int GetIpPost(char *pBuf, char *pIp, int *piPost);
int InitTCPClient(char *pIp, int iPost);
int TcpClientSend(int iFd, char *pBuf, int iLen);
int TcpRecv(int iFd, char *pBuf, int iLen);
void CloseFd(int iFd);


#endif
