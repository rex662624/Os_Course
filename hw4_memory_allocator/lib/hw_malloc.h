#ifndef HW_MALLOC_H
#define HW_MALLOC_H

#include <stddef.h>
#include <inttypes.h>
#include <unistd.h>
//typedef struct chunk_header* chunk_ptr_t;
typedef uint64_t chunk_size_t;
typedef uint64_t chunk_flag_t;


struct chunk_header {//總共40bytes

	struct chunk_header* prev;//8byte 指向前一個
	struct chunk_header* next;
	int chunk_size;           //int = 4bytes
	int prev_chunk_size;
	int prev_free_flag;
	int my_flag;
	chunk_size_t address;     //8bytes

};


extern void *hw_malloc(size_t bytes);
extern int hw_free(void *mem);
extern void *get_start_sbrk(void);

struct chunk_header bin[7];
#endif
