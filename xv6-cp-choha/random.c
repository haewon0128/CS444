#include "types.h"
#include "user.h"
#include "rand.h"

int main(void)
{

//#ifdef RAND_MAX
	int randNumber;
	randNumber = rrand();
	printf(1,"random number is: %d\n", randNumber);

	exit();
//#endif
}	
	

/*
#include <stdlib.h>
#include <stdio.h>
int
main(int argc, char *argv[])
{
         int j, r, nloops;
         unsigned int seed;

         if (argc != 3) {
             fprintf(stderr, "Usage: %s <seed> <nloops>\n", argv[0]);
             exit(EXIT_FAILURE);
         }

         seed = atoi(argv[1]);
         nloops = atoi(argv[2]);

         srand(seed);
         for (j = 0; j < nloops; j++) {
             r =  rand();
             printf("%d\n", r);
         }

            exit(EXIT_SUCCESS);
  
         }*/
