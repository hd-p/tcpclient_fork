#ifndef __FORK_PROCESS_H__
#define __FORK_PROCESS_H__

extern pid_t gpid;

int ForkProcess(int iNum);
void CloseChild(void);

#endif
