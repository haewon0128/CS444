#include "types.h"
#include "user.h"



int main(int argc, char *argv[])
{

	
	int nice = atoi(argv[1]);
	//int result;

	renice(nice, getpid());
	
	//if(result != 0)
	//{
	//	printf(0, "something wrong\n");
	//}
	
	exec(argv[2], &(argv[2]));

	exit();
	


}
