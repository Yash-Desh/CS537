#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]){
	int my_pid = getpid();
	printf(1, "Hello world from process id : %d\n", my_pid);
	exit();

}
	
