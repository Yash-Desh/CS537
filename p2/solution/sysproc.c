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
  // variables to store parameters parentbuffsize & childbufsize 
  // passed as arguments to the system call
  int pbuff;
  int cbuff;

  // Check if these arguments are present & within allocated address
  // space
  if(argint(2, &pbuff)<0 || argint(3, &cbuff)<0)
  {
    return -1;
  }

  // parentbuffsize or childbufsize is less than or equal to zero
  if(pbuff<=0 || cbuff<=0)
  {
     return -1;
  }

  // variables to store parameters parentbuff & childbuff
  // passed as arguments to the system call
  char *p;
  char *c;


  // check if childbuff & parentbuff are present & within allocated 
  // address space
  if(argstr(0, &p) < 0 || argstr(1, &c) < 0)
  {
    return -1;
  }

  // Null pointer handled
  if(p==(char*)0 || c == (char*)0)
  {
    return -1;
  }

  
  // determine length of the process names 
  int size_p = strlen(myproc()->parent->name);
  int size_c = strlen(myproc()->name);

  // check if size of parentbuff & childbuff is less than process name

  if(pbuff < size_p || cbuff < size_c)
  {
    return -1;
  }

  // copy the process names into parentbuff & childbuff
  safestrcpy(p, myproc()->parent->name, size_p+1);
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
