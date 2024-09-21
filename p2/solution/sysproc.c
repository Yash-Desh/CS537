#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

// our new system call handler function
int
sys_getparentname(void)
{
  int pbuff;
  int cbuff;
  if(argint(2, &pbuff)<0 || argint(3, &cbuff)<0)
  {
    return -1;
  }

  if(pbuff<=0 || cbuff<=0)
  {
     return -1;
  }

  char *p;
  char *c;
  if(argstr(0, &p) < 0 || argstr(1, &c) < 0)
  {
    return -1;
  }

  if(p==(char*)0 || c == (char*)0)
  {
    return -1;
  }

  int size_p = strlen(myproc()->parent->name);
  safestrcpy(p, myproc()->parent->name, size_p+1);
  int size_c = strlen(myproc()->name);
  safestrcpy(c, myproc()->name, size_c+1);
  return 1;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
