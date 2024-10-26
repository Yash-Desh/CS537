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

// settickets()
int sys_settickets(void)
{
  int tickets_new;

  // Check if these arguments are present & within allocated address
  // space
  if(argint(0, &tickets_new)<0)
  {
    return -1;
  }

  // decrement global_tickets
  global_tickets -= myproc()->tickets;

  if(tickets_new <= 0 || tickets_new > 32)
  {
    myproc()->tickets = 8;
  }
  else
  {
    myproc()->tickets = tickets_new;
  }

  // calculate stride
  int stride_new = STRIDE1/myproc()->tickets;

  // update remain
  myproc()->remain = (stride_new/myproc()->stride)*myproc()->remain;
  // update stride
  myproc()->stride = stride_new; 

  // increment global_tickets
  global_tickets += myproc()->tickets;

  // update global_stride
  global_stride = STRIDE1/global_tickets;
  
  return 1;
}
// getpinfo()
int sys_getpinfo(void)
{
  return 1;
}

// hello
int sys_hello(void)
{
  return myproc()->tickets;
}
