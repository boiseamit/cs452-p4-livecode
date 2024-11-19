#include "lab.h"
#include <errno.h>
#include <sys/mman.h>


size_t btok(size_t bytes)
{
	unsigned int count = 0;
	bytes--;
	while (bytes > 0) { bytes >>= 1; count++;}
	return count;
}

void buddy_init(struct buddy_pool *pool, size_t size)
{
	
	if (size == 0) {size = UINT64_C(1) << DEFAULT_K;}
	if (size < 0) {errno = ENOMEM;}

	pool->kval_m = btok(size);
	pool->numbytes = UINT64_C(1) << pool->kval_m;

		
	/* ask the kernel for a chunk of the specified size */
	pool->base = mmap(NULL, pool->numbytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pool->base == MAP_FAILED) {
        perror("BuddySystem: Could not allocate memory pool!");
		pool->base = 0; // null ptr
    }

    /* Initialize the lists of available blocks. The heads of
     * the lists are stored in the avail array. The head nodes don't
     * use the tag and kval fields. Note that the head nodes point to
     * themselves to denote an empty list.
     */
    for (int i = 0; i < pool->kval_m; i++) {
    	pool->avail[i].next =  &pool->avail[i];
    	pool->avail[i].prev =  &pool->avail[i]; 
    	pool->avail[i].kval = i;
		pool->avail[i].tag =  BLOCK_UNUSED;
    }

	/* Place all the memory as one free block in the appropriate list */
	pool->avail[pool->kval_m].next = pool->base;
	pool->avail[pool->kval_m].prev = pool->base;

	struct avail *ptr = (struct avail *) pool->base;
	ptr->tag = BLOCK_AVAIL;
	ptr->kval = pool->kval_m;
	ptr->next = &pool->avail[pool->kval_m];
	ptr->prev = &pool->avail[pool->kval_m];
}

void buddy_destroy(struct buddy_pool *pool) 
{
    int status = munmap(pool->base, pool->numbytes);
    if (status == -1) {
        perror("mmap-test: destroy failed!");
    }
}
	




