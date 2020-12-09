#include "types.h"
#include "user.h"



int main(int argc, char *argv[])
{

	int i=0;
	int nice = atoi(argv[1]);

	for (i = 2; i<argc; i++)
	{	
		//printf(1,"%d\n",atoi(argv[i])); 
		renice(nice, atoi(argv[i]));
	}
	exit();



}
