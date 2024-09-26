#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_NAME_LEN 256

int main(int argc, char* argv[]) {
  char parent_name[18];
  char child_name[20];

  if (getparentname(parent_name, child_name, 3, 3) < 0)
  {
    printf(2, "XV6_TEST_ERROR getparentname call failed!\n");
    exit();
  }

  printf(1, "XV6_TEST_OUTPUT Parent Name: %s,", parent_name);
  printf(1, " Child Name: %s\n", child_name);
  exit();
}
