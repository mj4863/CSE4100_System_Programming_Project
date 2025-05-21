#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * Team information
 ********************************************************/
team_t team = {
    /* Your student ID */
    "20212020",
    /* Your full name*/
    "Minjun Park",
    /* Your email address */
    "mj4863@sogang.ac.kr",
};

/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE   (1<<12) /* Extend heap by this amount (bytes) */
#define TOTAL_LIST   16      /* Number of segregated lists */
#define ALIGNMENT   8       /* Alignment requirement */
#define REALLOC_BUFFER  (1<<7) /* Buffer for realloc to reduce fragmentation */

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Free list macros */
#define PRED_PTR(bp)       ((char *)(bp))
#define SUCC_PTR(bp)       ((char *)(bp) + WSIZE)
#define PRED(bp)           (*(char **)(bp))
#define SUCC(bp)           (*(char **)(SUCC_PTR(bp)))

/* Global variables */
static char *heap_listp = 0;
static void* seg_list_1, *seg_list_2, *seg_list_3, *seg_list_4;
static void* seg_list_5, * seg_list_6, * seg_list_7, * seg_list_8;
static void* seg_list_9, * seg_list_10, * seg_list_11, * seg_list_12;
static void* seg_list_13, * seg_list_14, * seg_list_15, * seg_list_16;

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void insert_node(void *bp, size_t size);
static void delete_node(void *bp);

/*
 * mm_init - Initialize the malloc package.
 */
int mm_init(void)
{
    void** seg_lists[TOTAL_LIST] = {
		&seg_list_1, &seg_list_2, &seg_list_3, &seg_list_4,
		&seg_list_5, &seg_list_6, &seg_list_7, &seg_list_8,
		&seg_list_9, &seg_list_10, &seg_list_11, &seg_list_12,
		&seg_list_13, &seg_list_14, &seg_list_15, &seg_list_16,
	};

    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));
    heap_listp += (2 * WSIZE);

    int i = 0;
    while (i < TOTAL_LIST) {
        *seg_lists[i] = NULL;
        i++;
    }

    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    return 0;
}

/*
 * extend_heap - Extend the heap with a new free block.
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((bp = mem_sbrk(size)) == (void *)-1)
        return NULL;

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

/*
 * coalesce - Coalesce free blocks.
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {
        insert_node(bp, size);
        return bp;
    }

    if (prev_alloc && !next_alloc) {
        delete_node(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    } else if (!prev_alloc && next_alloc) {
        delete_node(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    } else {
        delete_node(PREV_BLKP(bp));
        delete_node(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    insert_node(bp, size);
    return bp;
}

/*
 * find_fit - Find a fit for a block with asize bytes.
 */
static void *find_fit(size_t asize)
{
    void** seg_lists[TOTAL_LIST] = {
		&seg_list_1, &seg_list_2, &seg_list_3, &seg_list_4,
		&seg_list_5, &seg_list_6, &seg_list_7, &seg_list_8,
		&seg_list_9, &seg_list_10, &seg_list_11, &seg_list_12,
		&seg_list_13, &seg_list_14, &seg_list_15, &seg_list_16,
	};

    void *bp;
    for (int i = 0; i < TOTAL_LIST; i++) {
        if ((asize <= 1 << (i + 4)) || (i == TOTAL_LIST - 1)) {
            bp = *seg_lists[i];
            while (bp && (asize > GET_SIZE(HDRP(bp)))) {
                bp = SUCC(bp);
            }
            if (bp)
                return bp;
        }
    }
    return NULL;
}

/*
 * place - Place block of asize bytes at start of free block bp and split if remainder would be at least minimum block size.
 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= (2 * DSIZE)) {
        delete_node(bp);
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        coalesce(bp);
    } else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
        delete_node(bp);
    }
}

/*
 * mm_malloc - Allocate a block with at least size bytes of payload.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;

    if (size == 0)
        return NULL;

    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Free a block.
 */
void mm_free(void *bp)
{
    if (bp == NULL)
        return;

    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * mm_realloc - Reallocate a block to have at least size bytes of payload.
 */
void *mm_realloc(void *ptr, size_t size)
{
    if ((int)size < 0)
        return NULL;
    else if ((int)size == 0) {
        mm_free(ptr);
        return NULL;
    } else if (size > 0 && ptr == NULL) {
        return mm_malloc(size);
    }

    void *new_ptr = ptr;
    size_t new_size = size + REALLOC_BUFFER;
    int remainder;
    size_t extendsize;
    size_t block_size = GET_SIZE(HDRP(ptr));

    if (new_size <= block_size) {
        return ptr;
    } else {
        if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr))) || !GET_SIZE(HDRP(NEXT_BLKP(ptr)))) {
            remainder = block_size + GET_SIZE(HDRP(NEXT_BLKP(ptr))) - new_size;
            if (remainder < 0) {
                extendsize = MAX(-remainder, CHUNKSIZE);
                if (extend_heap(extendsize / WSIZE) == NULL)
                    return NULL;
                remainder += extendsize;
            }
            delete_node(NEXT_BLKP(ptr));
            PUT(HDRP(ptr), PACK(new_size + remainder, 1));
            PUT(FTRP(ptr), PACK(new_size + remainder, 1));
        } else {
            new_ptr = mm_malloc(new_size);
            memcpy(new_ptr, ptr, MIN(size, new_size));
            mm_free(ptr);
        }
    }
    return new_ptr;
}

/*
 * insert_node - Insert a free block into the appropriate segregated list.
 */
static void insert_node(void *bp, size_t size)
{
    void** seg_lists[TOTAL_LIST] = {
		&seg_list_1, &seg_list_2, &seg_list_3, &seg_list_4,
		&seg_list_5, &seg_list_6, &seg_list_7, &seg_list_8,
		&seg_list_9, &seg_list_10, &seg_list_11, &seg_list_12,
		&seg_list_13, &seg_list_14, &seg_list_15, &seg_list_16,
	};

    int list = 0;
    void *search_ptr = bp;
    void *insert_ptr = NULL;

    while ((list < TOTAL_LIST - 1) && (size > 1)) {
        size >>= 1;
        list++;
    }

    search_ptr = *seg_lists[list];
    while ((search_ptr != NULL) && (size > GET_SIZE(HDRP(search_ptr)))) {
        insert_ptr = search_ptr;
        search_ptr = SUCC(search_ptr);
    }

    if (search_ptr != NULL) {
        if (insert_ptr != NULL) {
            SUCC(bp) = search_ptr;
            PRED(search_ptr) = bp;
            SUCC(insert_ptr) = bp;
            PRED(bp) = insert_ptr;
        } else {
            SUCC(bp) = search_ptr;
            PRED(search_ptr) = bp;
            PRED(bp) = NULL;
            *seg_lists[list] = bp;
        }
    } else {
        if (insert_ptr != NULL) {
            SUCC(insert_ptr) = bp;
            PRED(bp) = insert_ptr;
            SUCC(bp) = NULL;
        } else {
            SUCC(bp) = NULL;
            PRED(bp) = NULL;
            *seg_lists[list] = bp;
        }
    }
}

/*
 * delete_node - Delete a free block from the segregated list.
 */
static void delete_node(void *bp)
{
    void** seg_lists[TOTAL_LIST] = {
		&seg_list_1, &seg_list_2, &seg_list_3, &seg_list_4,
		&seg_list_5, &seg_list_6, &seg_list_7, &seg_list_8,
		&seg_list_9, &seg_list_10, &seg_list_11, &seg_list_12,
		&seg_list_13, &seg_list_14, &seg_list_15, &seg_list_16,
	};

    int list = 0;
    size_t size = GET_SIZE(HDRP(bp));

    while ((list < TOTAL_LIST - 1) && (size > 1)) {
        size >>= 1;
        list++;
    }

    if (PRED(bp) != NULL) {
        SUCC(PRED(bp)) = SUCC(bp);
    } 
    else {
        *seg_lists[list] = SUCC(bp);
    }
    if (SUCC(bp) != NULL) {
        PRED(SUCC(bp)) = PRED(bp);
    }
}
