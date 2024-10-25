#include "types.h"
#include "stat.h"
#include "user.h"
int main(void) 
{
  printf(1, "The process ID is: %d\n", getpid());
  printf(1, "The process ID from my hello sys_call is: %d\n", hello());
  exit();
}