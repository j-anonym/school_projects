// xbucht28

/**
 * Implementace My MALloc
 * Demonstracni priklad pro 1. ukol IPS/2018
 * Ales Smrcka
 */

#include "mmal.h"
#include <sys/mman.h> // mmap
#include <stdbool.h> // bool
#include <assert.h> // assert
#include <memory.h>

#ifdef NDEBUG
/**
 * The structure header encapsulates data of a single memory block.
 *   ---+------+----------------------------+---
 *      |Header|DDD not_free DDDDD...free...|
 *   ---+------+-----------------+----------+---
 *             |-- Header.asize -|
 *             |-- Header.size -------------|
 */
typedef struct header Header;
struct header {

    /**
     * Pointer to the next header. Cyclic list. If there is no other block,
     * points to itself.
     */
    Header *next;

    /// size of the block
    size_t size;

    /**
     * Size of block in bytes allocated for program. asize=0 means the block 
     * is not used by a program.
     */
    size_t asize;
};

/**
 * The arena structure.
 *   /--- arena metadata
 *   |     /---- header of the first block
 *   v     v
 *   +-----+------+-----------------------------+
 *   |Arena|Header|.............................|
 *   +-----+------+-----------------------------+
 *
 *   |--------------- Arena.size ---------------|
 */
typedef struct arena Arena;
struct arena {

    /**
     * Pointer to the next arena. Single-linked list.
     */
    Arena *next;

    /// Arena size.
    size_t size;
};

#define PAGE_SIZE (128*1024)

#endif // NDEBUG

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif

static void hdr_ctor(Header *hdr, size_t size);


Arena *first_arena = NULL;

/**
 * Return size alligned to PAGE_SIZE
 */
static
size_t allign_page(size_t size) {
    return ((size + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE);
}

/**
 * Allocate a new arena using mmap.
 * @param req_size requested size in bytes. Should be alligned to PAGE_SIZE.
 * @return pointer to a new arena, if successfull. NULL if error.
 * @pre req_size > sizeof(Arena) + sizeof(Header)
 */

/**
 *   +-----+------------------------------------+
 *   |Arena|....................................|
 *   +-----+------------------------------------+
 *
 *   |--------------- Arena.size ---------------|
 */
static
Arena *arena_alloc(size_t req_size) {
    assert(req_size > sizeof(Arena) + sizeof(Header));
    Arena *toBeAlloc;
    toBeAlloc = (Arena *) mmap(NULL, req_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (toBeAlloc == MAP_FAILED) {
        return NULL;
    }
    toBeAlloc->next = NULL;
    toBeAlloc->size = req_size;
    Header *newHead = (Header *) &toBeAlloc[1];;
    hdr_ctor(newHead, req_size - sizeof(Arena) - sizeof(Header));
    if (first_arena)
        newHead->next = (Header *) &first_arena[1];
    else
        newHead->next = newHead;
    return toBeAlloc;
}

/**
 * Appends a new arena to the end of the arena list.
 * @param a     already allocated arena
 */
static
void arena_append(Arena *a) {
    if (a == NULL)
        return;

    if (first_arena == NULL) {
        first_arena = a;
        return;
    }

    Arena *tmp = first_arena;
    while (1) {
        if (tmp->next == NULL) {
            tmp->next = a;
            break;
        }
        tmp = tmp->next;
    }
}

/**
 * Header structure constructor (alone, not used block).
 * @param hdr       pointer to block metadata.
 * @param size      size of free block
 * @pre size > 0
 */
/**
 *   +-----+------+------------------------+----+
 *   | ... |Header|........................| ...|
 *   +-----+------+------------------------+----+
 *
 *                |-- Header.size ---------|
 */
static
void hdr_ctor(Header *hdr, size_t size) {
    if (hdr == NULL)
        return;
    hdr->size = size;
    hdr->asize = 0;
}

/**
 * Checks if the given free block should be split in two separate blocks.
 * @param hdr       header of the free block
 * @param size      requested size of data
 * @return true if the block should be split
 * @pre hdr->asize == 0
 * @pre size > 0
 */
static
bool hdr_should_split(Header *hdr, size_t size) {
    assert(hdr->asize == 0);
    assert(size > 0);
    return (hdr->size > (size + 2 * sizeof(Header)));
}

/**
 * Splits one block in two.
 * @param hdr       pointer to header of the big block
 * @param req_size  requested size of data in the (left) block.
 * @return pointer to the new (right) block header.
 * @pre   (hdr->size >= req_size + 2*sizeof(Header))
 */
/**
 * Before:        |---- hdr->size ---------|
 *
 *    -----+------+------------------------+----
 *         |Header|........................|
 *    -----+------+------------------------+----
 *            \----hdr->next---------------^
 */
/**
 * After:         |- req_size -|
 *
 *    -----+------+------------+------+----+----
 *     ... |Header|............|Header|....|
 *    -----+------+------------+------+----+----
 *             \---next--------^  \--next--^
 */
static
Header *hdr_split(Header *hdr, size_t req_size) {
    assert((hdr->size >= req_size + 2 * sizeof(Header)));
    Header *newHeader = (Header *) ((long) hdr + sizeof(Header) + req_size);
    Header *tmp = hdr->next;
    hdr->next = newHeader;
    newHeader->next = tmp;
    hdr_ctor(newHeader, hdr->size - req_size - sizeof(Header));
    hdr->size = req_size;
    return newHeader;
}

/**
 * Detect if two adjacent blocks could be merged.
 * @param left      left block
 * @param right     right block
 * @return true if two block are free and adjacent in the same arena.
 * @pre left->next == right
 * @pre left != right
 */
static
bool hdr_can_merge(Header *left, Header *right) {
    assert(left->next == right);
    assert(left != right);

    return (left->asize == 0 && right->asize == 0);
}

/**
 * Merge two adjacent free blocks.
 * @param left      left block
 * @param right     right block
 * @pre left->next == right
 * @pre left != right
 */
static
void hdr_merge(Header *left, Header *right) {
    assert(left->next == right);
    assert(left != right);
    left->next = right->next;
    left->size = left->size + sizeof(Header) + right->size;
}


/**
 * Finds the first free block that fits to the requested size.
 * @param size      requested size
 * @return pointer to the header of the block or NULL if no block is available.
 * @pre size > 0
 */
static
Header *first_fit(size_t size) {
    Arena *temp = first_arena;

    do {
        Header *firstH = (Header *) (&first_arena[1]);

        long address = (long) firstH;
        while (1) {
            if (size < (firstH->size - firstH->asize)) {
                return firstH;
            }
            if ((long) firstH->next == address) { //there wasn't found enough space
                Arena *new_Arena = arena_alloc(allign_page(size + sizeof(Header) + sizeof(Arena)));
                if (new_Arena == NULL)
                    return NULL;

                arena_append(new_Arena);
                firstH->next = (Header *) (&new_Arena[1]);

            }

            firstH = firstH->next;
        }
        temp = (Arena *) ((long) temp + temp->size);

    } while (temp);
    return NULL;

}

/**
 * Search the header which is the predecessor to the hdr. Note that if 
 * @param hdr       successor of the search header
 * @return pointer to predecessor, hdr if there is just one header.
 * @pre first_arena != NULL
 * @post predecessor->next == hdr
 */
static
Header *hdr_get_prev(Header *hdr) {
    assert(first_arena != NULL);
    bool secondloop = false;
    long address = (long) hdr;

    while (1) {
        if ((long) hdr->next == address)
            return hdr;

        if ((long) hdr == address) {
            if (secondloop) {
                return hdr;
            }
            secondloop = true;
        }
        hdr = hdr->next;
    }

}

/**
 * Allocate memory. Use first-fit search of available block.
 * @param size      requested size for program
 * @return pointer to allocated data or NULL if error or size = 0.
 */
void *mmalloc(size_t size) {
    if (first_arena == NULL) {
        Arena *newArena = arena_alloc(allign_page(size + sizeof(Arena) + sizeof(Header)));

        if (newArena == NULL)
            return NULL;

        arena_append(newArena);
    }

    Header *h = first_fit(size);


    if (h == NULL)
        return NULL;

    if (hdr_should_split(h, size))
        hdr_split(h, size);

    h->asize = size;

    return (void *) (long) h + sizeof(Header);
}

/**
 * Free memory block.
 * @param ptr       pointer to previously allocated data
 * @pre ptr != NULL
 */
void mfree(void *ptr) {
    assert(ptr != NULL);
    Header *h = (Header *) ((long) ptr - sizeof(Header));

    h->asize = 0;
    if (hdr_can_merge(h, h->next)) {
        hdr_merge(h, h->next);
    }
    if (hdr_get_prev(h) < h) {
        if (hdr_can_merge(hdr_get_prev(h), h)) {
            hdr_merge(hdr_get_prev(h), h);
        }
    }

 /*   Header *tmp = (Header *) (&first_arena[1]);
 ---------------Deleting Arenas is not required in this project, but works fine-----------------
    //check if any arena could be deleted
    while (first_arena) {
        if (first_arena->next) {
            if (first_arena->size == tmp->size + tmp->asize + sizeof(Arena) + sizeof(Header)) {
                Arena *tmpArena = first_arena->next;
                hdr_get_prev(tmp)->next = tmp->next; //bind list, becouse Header will be deleted
                if (munmap(first_arena, first_arena->size) == -1)
                    return;
                first_arena = tmpArena;
                tmp = (Header *) (&first_arena[1]);
                continue;
            }
        }
        break;
    }*/
}

/**
 * Reallocate previously allocated block.
 * @param ptr       pointer to previously allocated data
 * @param size      a new requested size. Size can be greater, equal, or less
 * then size of previously allocated block.
 * @return pointer to reallocated space or NULL if size equals to 0.
 */
void *mrealloc(void *ptr, size_t size) {

    Header *head = (Header *) ((long) ptr - sizeof(Header));
    void *newData;

    if (size == 0) {
        mfree(ptr);
        return NULL;
    } else if (ptr == NULL) {
        return mmalloc(size);
    } else if (size <= head->asize) {
        size_t temp = size;
        head->asize = 0;
        if (hdr_should_split(head, temp))
            hdr_split(head, temp);
        head->asize = temp;
        return ptr;
    } else {
        newData = mmalloc(size);
        memcpy(newData, ptr, head->asize);
        mfree(ptr);
    }
    return newData;
}
