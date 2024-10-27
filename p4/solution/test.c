#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"
#include "pstat.h"
int main(void) 
{
  printf(1, "The process ID is: %d\n", getpid());
  printf(1, "The process tickets are: %d\n", hello());
  settickets(-3);
  printf(1, "The process tickets are: %d\n", hello());

  struct pstat ps;
  if (getpinfo(&ps) != 0) {
      printf(1, "Failed to get process info\n");
      exit();
    }
  for (int i = 0; i < NPROC; i++) 
  {
    printf(1, "%d\t%d\t%d\t%d\t%d\n",
          ps.pid[i],
          ps.tickets[i],
          ps.pass[i],
          ps.stride[i],
          ps.rtime[i]);
    }

  exit();
}