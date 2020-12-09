#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>
#include <values.h>
#include <signal.h>
#include "beavalloc.h"


#define ALLOC 1024




struct list_block {
	size_t size;
	void* dataAddr;
	struct list_block *next;
	struct list_block *prev;
	int isFree;
	size_t capacity;
	

};

static uint verbose = FALSE;

//int errno;
void* lower_mem_bound = NULL;
void* upper_mem_bound = NULL;
static struct list_block *base = NULL;
static size_t headersize = sizeof(struct list_block);		
static struct list_block *first = NULL;
void *beavalloc(size_t size)
{	

	if(size<=0)
	{
		return NULL;
	}
	if(first ==NULL)
	{	
		if(verbose == TRUE)
		{
			fprintf(stderr, "first time allocating\n");
		}
		base = sbrk(0);
		lower_mem_bound = base;
	
		if(headersize+size<=ALLOC)
		{	
			first = sbrk(ALLOC);
			if(first == (void*)-1)
			{
				errno = ENOMEM;
				return NULL;
			}	
			first->capacity = ALLOC - headersize;
		}
		else if(headersize+size>ALLOC)
		{
			/*int x = ((size +headersize)/1024)+1;
			
			first = sbrk(ALLOC*x);
			*/
			
			first = sbrk(size+headersize);
			if(first == (void *)-1)
			{
				errno = ENOMEM;
				return NULL;
			}
			first->capacity = size;
			
			
		}
		


		first->prev = NULL;
		first->next = NULL;
		first->isFree = 0;
		first->size = size;

		first->dataAddr = (void *)((unsigned long)first + (unsigned long)headersize);
		
		upper_mem_bound = sbrk(0);


		if(verbose == TRUE)
                {
                        fprintf(stderr, "returning point, %p size of %zu \n", first->dataAddr, size);
                }
	
		return first->dataAddr;
	
	}

	else if(first !=NULL)
	{
		struct list_block *curr=NULL;
		struct list_block *addingnew = NULL;
	
		curr= first;
		//for(curr=first; curr->next!=NULL; curr=curr->next)
		while(curr != NULL)
		{

			if(curr->isFree ==1 && curr->capacity >= size)
			{	
				curr->size = size;
				curr->isFree = 0;
				if(verbose == TRUE)
				{
					fprintf(stderr,"using freed block\n");
					fprintf(stderr,"returning point, %p size of %zu \n", curr->dataAddr,size);
				}
				return curr->dataAddr;
			}
			else if(curr->isFree==0 && (curr->capacity-curr->size)>=(size+headersize))
			{
				struct list_block *newbl=NULL;
				newbl = (void*)((unsigned long)curr->dataAddr+curr->size);
				
				newbl->prev = curr;
				newbl->next = curr->next;
				newbl->isFree = 0;
				newbl->dataAddr = (void *)((unsigned long)newbl+headersize);
				newbl->capacity = curr->capacity - curr->size - headersize;
				newbl->size = size;

				curr->next = newbl;
				curr->capacity = curr->size;
					
				if(newbl->next != NULL)
				{
					newbl->next->prev= newbl;
				}



				if(verbose == TRUE)
                                {
                                        fprintf(stderr,"splitting memory block\n");
                                        fprintf(stderr,"returning point, %p size of %zu \n", newbl->dataAddr,size);
                                }
				
				return newbl->dataAddr;
			}
			if(curr->next ==NULL)
			{break;}

			curr = curr ->next;
			

		}
		
		if(headersize+size<=ALLOC)
		{
			addingnew = sbrk(ALLOC);
			if(addingnew == (void*)-1)
			{
				errno = ENOMEM;
				return NULL;
			}
			addingnew->capacity = ALLOC-headersize;
		}	
		else if(headersize+size>ALLOC)
		{
			//int x = ((size+headersize)/ALLOC)+1;
			addingnew = sbrk(size+headersize);
			if(addingnew==(void*)-1)
			{
				errno = ENOMEM;
				return NULL;
			}
			addingnew->capacity = size;
		}
		
		curr->next = addingnew;
		addingnew->next = NULL;
		addingnew->prev =curr;
		addingnew->isFree = 0;
		addingnew->dataAddr = (void *)((unsigned long)addingnew+headersize);
		addingnew->size = size;	
		
		upper_mem_bound = sbrk(0);


		if(verbose == TRUE)
                {
                	fprintf(stderr,"sbrk new block\n");
                        fprintf(stderr,"returning point, %p size of %zu \n", addingnew->dataAddr,size);
                }



		return addingnew->dataAddr;


	}

	return NULL;

}



void beavalloc_reset(void)
{
	if(verbose == TRUE)
        {
        	fprintf(stderr,"reset everything!!\n");
                      
        }

	brk(base);
	lower_mem_bound = NULL;
	upper_mem_bound = NULL;
	first = NULL;
	//base = NULL;


}


void beavfree(void *ptr)
{
	
	
	struct list_block *curr;
	if(ptr ==NULL)
	{
		if(verbose == TRUE)
                {
                	fprintf(stderr,">>>ptr is null.. nothing to free\n");
                }
	
		return;
	}
	
	curr = (void *)((unsigned long)ptr - headersize);




	if(verbose == TRUE)
        {
                fprintf(stderr,">>>beavfree entry: ptr = %p curr = %p\n", ptr, curr);
        }





	if(curr->isFree == 1)
	{
		if(verbose == TRUE)
                {
                        fprintf(stderr,">>>block is already freed.. nothing to free: ptr = %p\n", ptr);
                }
	

		
		return;
	}
 	
	curr->isFree = 1;
	curr->size = 0;
	if(curr->prev!=NULL && curr->next!=NULL)
	{
		if(curr->prev->isFree == 1&&curr->next->isFree==1)
		{
			struct list_block *p = curr->prev;
			struct list_block *n = curr->next;

			p->capacity = p->capacity + curr->capacity+n->capacity+headersize+headersize;
			p->next = n->next;
			p->size = 0;
			if(p->next != NULL)
			{
				p->next->prev = p;
			}
			n = NULL;
			curr = NULL;


			if(verbose == TRUE)
                	{
                        	fprintf(stderr,"coalesce right and left. New block: %p\n", p->dataAddr);
                	}




			return ;
		}
		else if(curr->prev->isFree==1&&curr->next->isFree!=1)
		{
			struct list_block *p = curr->prev;

			p->capacity = p->capacity +curr->capacity+headersize;
			p->next = curr->next;
			curr->next->prev = p;
			p->size = 0;
			curr = NULL;


			if(verbose == TRUE)
                        	{
                                	fprintf(stderr,"coalesce left. New block: %p\n", p->dataAddr);
                        	}


			return;
		}
		
		else if(curr->prev->isFree!=1&&curr->next->isFree==1)
		{
			struct list_block *n = curr->next;
		
			curr->capacity = curr->capacity+n->capacity+headersize;
			curr->next = n->next;
			curr->size = 0;
			if(n->next != NULL)
			{
				curr->next->prev = curr;
			}
			n = NULL;

			if(verbose == TRUE)
                        {
                                fprintf(stderr,"coalesce left. New block: %p\n", curr->dataAddr);
                        }


			return;
		
		}	
	}
	else if(curr->prev!=NULL && curr->next ==NULL)
	{
		if(curr->prev->isFree ==1)
		{
			struct list_block *p = curr->prev;

			p->capacity = p->capacity + curr->capacity+headersize;
			p->next = curr->next;
			p->size = 0;
			curr = NULL;


			if(verbose == TRUE)
                        {
                                fprintf(stderr,"coalesce left. New block: %p\n", p->dataAddr);
                        }

			return;
		}
		else if(curr->prev->isFree==0)
		{
			return;
		}
	}
	else if(curr->prev==NULL && curr->next != NULL)
	{
		if(curr->next->isFree==1)
		{
			struct list_block *n = curr->next;
			curr->capacity = curr->capacity+n->capacity+headersize;
			curr->next = n->next;
			curr->size = 0;
			if(curr->next != NULL)
			{
				curr->next->prev = curr;
			}
			n=NULL;


			if(verbose == TRUE)
                	{
                        	fprintf(stderr,"coalesce left. New block: %p\n", curr->dataAddr);
                	}


			return;
		}
		else if(curr->next->isFree==0)
		{
			return;
		}
	}
	else if(curr->prev ==NULL&&curr->next ==NULL)
	{
		return;
	}


	

	return;
}

void *beavcalloc(size_t nmemb, size_t size)
{

	
	void *ptr;
	if(verbose == TRUE)
        {
                fprintf(stderr,"beavcalloc entry\n");
        }


	if(nmemb ==0 || size==0)
	{
		return NULL;
	}	
	
	ptr = beavalloc(size*nmemb);
	memset(ptr,0,size*nmemb);





	return ptr;	


}






void beavalloc_set_verbose(uint8_t a)
{
	if(a==FALSE)
	{
		verbose = FALSE;
	}
	else
	{
		verbose = TRUE;
		//printf("");
	}
	return;
}


void*beavrealloc(void *ptr, size_t size)
{

	
	if(verbose == TRUE)
        {
                fprintf(stderr,"beavrealloc entry\n");
        }
	
	if(size == 0)
	{
		return NULL;
	}
	
	

	if(ptr == NULL)
	{
	

		size_t tsize = size*2;

		if(verbose == TRUE)
        	{
                	fprintf(stderr,"ptr is null.... doubling size\n");
        	}


	
		return beavalloc(tsize);
	}
	
	else
	{
	
	
		struct list_block *new = NULL;
		struct list_block *curr = NULL;

		curr = ((void *)(unsigned long)ptr - headersize);

		if(curr->capacity>=size)
		{
			curr->size = size;
			return ptr;

		} 
		else if(curr->capacity < size)
		{
			new = beavalloc(size);


			if(verbose == TRUE)
        		{
               			fprintf(stderr,"copying data from old block to new block\n");
        		}

			memcpy(new, ptr,curr->size);
			beavfree(ptr);

			return new;			
			
		}
	}

	return NULL;

}











void beavalloc_dump(uint leaks_only)
{


    struct list_block *curr = NULL;
    uint i = 0;
    uint leak_count = 0;
    uint user_bytes = 0;
    uint capacity_bytes = 0;
    uint block_bytes = 0;
    uint used_blocks = 0;
    uint free_blocks = 0;

    if (leaks_only) {
        fprintf(stderr, "heap lost blocks\n");
    }
    else {
        fprintf(stderr, "heap map\n");
    }
    fprintf(stderr
            , "  %s\t%s\t%s\t%s\t%s"
            "\t%s\t%s\t%s\t%s\t%s\t%s"
            "\n"
            , "blk no  "
            , "block add "
            , "next add  "
            , "prev add  "
            , "data add  "

            , "blk off  "
            , "dat off  "
            , "capacity "
            , "size     "
            , "blk size "
            , "status   "
        );
 for (curr = first , i = 0; curr != NULL; curr = curr->next, i++) {
   if (leaks_only == FALSE || (leaks_only == TRUE && curr->isFree == FALSE)) {
            fprintf(stderr
                    , "  %u\t\t%9p\t%9p\t%9p\t%9p\t%u\t\t%u\t\t"
                      "%u\t\t%u\t\t%u\t\t%s\t%c\n"
                    , i
                    , curr
                    , curr->next
                    , curr->prev
                    , curr->dataAddr
                    , (unsigned) ((void *) curr - lower_mem_bound)
                    , (unsigned) ((void *) curr->dataAddr - lower_mem_bound)
                    , (unsigned) curr->capacity
                    , (unsigned) curr->size
                    , (unsigned) (curr->capacity + sizeof(struct list_block))
                    , curr->isFree ? "free  " : "in use"
                    , curr->isFree ? '*' : ' '
                );
            user_bytes += curr->size;
            capacity_bytes += curr->capacity;
            block_bytes += curr->capacity + sizeof(struct list_block);
            if (curr->isFree == FALSE && leaks_only == TRUE) {
                leak_count++;
            }
            if (curr->isFree == TRUE) {
                free_blocks++;
            }
            else {
                used_blocks++;
            }
        }
	//curr = curr->next;
    }

   if (leaks_only) {
        if (leak_count == 0) {
            fprintf(stderr, "  *** No leaks found!!! That does NOT mean no leaks are possible. ***\n");
        }
        else {
            fprintf(stderr
                    , "  %s\t\t\t\t\t\t\t\t\t\t\t\t"
                      "%u\t\t%u\t\t%u\n"
                    , "Total bytes lost"
                    , capacity_bytes
                    , user_bytes
                    , block_bytes
                );
        }
    }
    else {
        fprintf(stderr
                , "  %s\t\t\t\t\t\t\t\t\t\t\t\t"
                "%u\t\t%u\t\t%u\n"
                , "Total bytes used"
                , capacity_bytes
                , user_bytes
                , block_bytes
            );
        fprintf(stderr, "  Used blocks: %u  Free blocks: %u  "
             "Min heap: %p    Max heap: %p\n"
               , used_blocks, free_blocks
               , lower_mem_bound, upper_mem_bound
            );
    }
    

     
} 
       
