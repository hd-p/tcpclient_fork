#include <stdio.h>
#include <stdlib.h>

#include "fork_process.h"
pid_t gpid;

int ForkProcess(int iNum)
{
	if(iNum <= 0)
		return ;
	int iInx = 0;
	
	
	for(; iInx < iNum; iInx++)
	{
		gpid = fork();
		if(gpid == 0)
			break;
		if(gpid < 0)
		{
			printf("Can't Creat child process.error!!!!\r\n");
			break;
		}
	}
	

	sleep(1);
	printf("process %d\r\n", iInx);
	return iInx;
}

void CloseChild(void)
{
	if(gpid == 0)
		exit(0);
}


// int main()
// {
// 	int iInx = 0;
	
// 	ForkProcess(5);
// }