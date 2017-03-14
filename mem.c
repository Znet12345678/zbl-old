/*
*Zach's memory allocator
*(c)2017 Zachary James Schlotman
*/
#include "lib.h"
#include "mem.h"
#include "kern.h"
#include <stdint.h>
void mem_init(){
	uint8_t *pntr = (uint8_t *)0x01000000;
	struct mem_part *mem = (struct mem_part *)0x01000000;
	mem->alloc = 0;
	mem->size = 0;
	mem->complete = 0;
	mem->nxt = (struct mem_part *)0;
	mem->begin = (uint8_t *)0;
	mem->end = (uint8_t *)0;
}

void init_mem(void *pntr,unsigned long size){
	uint8_t *u8Pntr = (uint8_t*)pntr;
	for(int i = 0; i < size;i++,*u8Pntr++)
		*u8Pntr = 0;
}
struct mem_part *find_n(uint8_t *pntr,uint32_t n){
	struct mem_part *ret = (struct mem_part *)pntr;
	for(int i = 0; i < n;i++){
		ret = ret + ret->size + sizeof(struct mem_part);
	}
	return ret;
}
struct mem_part *find_free(uint8_t *pntr,unsigned long size){
	unsigned long alloc = 0;
	struct mem_part *mem = (struct mem_part *)pntr;
	if(mem->alloc == 0){
		if(mem->size == 0 || mem->size > size){
			mem->alloc = 1;
			mem->size = size;
			mem->complete = 1;
			mem->nxt = 0;
			mem->begin = (uint8_t*)(mem + sizeof(struct mem_part));
			mem->end = mem->begin + size;
			alloc+=size;
		}else{
			mem->alloc = 1;
			mem->complete = 0;
			mem->n++;
			mem->nxt = find_free(pntr + sizeof(struct mem_part) + size,size - alloc);
			mem->begin = (uint8_t*)(mem + sizeof(struct mem_part));
			mem->end = (uint8_t*)(mem + sizeof(struct mem_part) + mem->size);
			alloc+=mem->size;
		}
	}else{
		mem->n++;
		mem = find_free(pntr + sizeof(struct mem_part) + mem->size,size - alloc);
	}
	return mem;
}

void *malloc(unsigned long rsize){
	uint8_t *_pntr = (uint8_t *)0x01000000;
	struct mem_part *mem = find_free(_pntr,rsize);
	int allocated = 0;
	uint8_t *ret = mem->begin - 0x01000000 + 0x00000500 - 1;
	*ret = mem->n;
	*ret++;
	for(int i = 0; i < mem->size;i++)
		ret[i] = 0;
	allocated+=mem->size;
	mem = mem->nxt;
	while(mem != (struct mem_part *)0 && allocated< rsize){
		int i = 0;
		while(i < mem->size && allocated < rsize){
			ret[allocated] = *mem->begin;
			*mem->begin++;
			allocated++;
		}
		mem = mem->nxt;
	}
//	for(int i = 0; i < rsize;i++)
//		ret[i] = 0;
	/*int allocated = 0;
	int i = 0;
	while(mem != (struct mem_part *)0 && allocated < rsize){
		uint8_t *pntr = (uint8_t *)mem->begin;
		uint8_t *init = (uint8_t *)mem->begin;
		init_mem(pntr,mem->size);
		i = 0;
		while(i < mem->size && allocated < rsize){
			*ret = *pntr;
			*ret++;
			*pntr++;
			allocated++;
			i++;
		}
		mem = mem->nxt;
	}*/
	return (void*)ret;
}
void free(void *v){
	uint8_t *pntr = (uint8_t*)0x01000000;
	uint8_t *_pntr = (uint8_t*)v;
	struct mem_part *mem = find_n(pntr,*(_pntr - 1));
	mem->alloc = 0;
	while(mem != 0){
		mem->alloc = 0;
		mem = mem->nxt;
	}
}
void *blmalloc(unsigned long size){
	return malloc(size);
}
