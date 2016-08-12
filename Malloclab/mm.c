/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
//team_t team = {
//    /* Team name */
//    "ateam",
//    /* First member's full name */
//    "Harry Bovik",
//	
//    /* First member's email address */
//    "bovik@cs.cmu.edu",
//    /* Second member's full name (leave blank if none) */
//    "",
//    /* Second member's email address (leave blank if none) */
//    ""
//};


#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

#define MAX(x,y) ((x)>(y)?(x):(y))
//将申请字节数目和有效位并入头部或者脚部，头部脚部为4个字节大小，且字节数目默认偶数，故头部脚部的最低三位不作为记录字节数目
#define PACK(size,alloc) ((size)|(alloc))
//注意p是分到mem_sbrk返回的指针，brk是p向后移动4字节的实际指针
//将指针的开始4个字节作为unsigned显示   
#define GET(p) (*(unsigned *)(p))
#define PUT(p,val) (*(unsigned *)(p))=(val)

//获取申请字节数目
#define GET_SIZE(p) (GET(p)|~0x7)
#define GET_ALLOC(p) (GET(p)|0x1)

//从实际块指针中获取头部与脚部的位置
#define HEAD_ADDRESS(bp) ((char*)bp-WSIZE)
#define FOOT_ADDRESS(bp) ((char*)bp+GET_SIZE(HEAD_ADDRESS(bp))-DSIZE)
//从块指针中获取前一个块指针以及后一个块指针
#define NEXT_ADDRESS(bp) ((char*)bp+GET_SIZE(HEAD_ADDRESS(bp)))
#define PREV_ADDRESS(bp) ((char*)bp-GET_SIZE(HEAD_ADDRESS(bp)))


static char * heap_listp; //指向第一个块的指针i



/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if((heap_listp=mem_sbrk(4*WSIZE))==((void *)-1)){
       return -1; 
    }
    PUT(heap_listp,0);
    
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{   
    
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














