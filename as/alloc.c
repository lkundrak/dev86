
#include "syshead.h"
#include "const.h"
#include "type.h"
#include "align.h"

PRIVATE char NOMEMEORY[] = "Cannot allocate sufficient memory";

#ifdef USE_FIXED_HEAP
PRIVATE char *heapend;		/* end of free space for symbol list */
PRIVATE char *heapptr;		/* next free space in symbol list */
#endif

#ifndef USE_FIXED_HEAP
PRIVATE char tempbuf[2048];
#endif

void
init_heap()
{
#ifdef USE_FIXED_HEAP
#ifndef USERMEM
#define USERMEM 0xAC00U
#endif

#ifdef __AS386_16__
    int stk;
    heapptr = sbrk(0);
    heapend = ((char*)&stk) - STAKSIZ - 16;
    brk(heapend);
    if(sbrk(0) != heapend)
       as_abort(NOMEMEORY);
#else
#ifdef SOS_EDOS
    heapend = stackreg() - STAKSIZ;
#else
    heapptr = malloc(USERMEM);
    heapend = heapptr + USERMEM;
    if (heapptr == 0)
	as_abort(NOMEMEORY);
#endif
#endif
#endif
}

void * temp_buf()
{
#ifdef USE_FIXED_HEAP
    return heapptr;
#else
    return tempbuf;
#endif
}

void * 
asalloc(size)
unsigned int size;
{
    void * rv;
#ifdef USE_FIXED_HEAP
    align(heapptr);
    if (heapptr+size < heapend)
    {
        rv = heapptr;
        heapptr += size;
    }
    else
       rv = 0;
#else
    rv = malloc(size);
#endif
    if (rv == 0 && size) as_abort(NOMEMEORY);
    return rv;
}


void * 
asrealloc(oldptr, size)
void * oldptr;
unsigned int size;
{
    void * rv;
#ifdef USE_FIXED_HEAP
    if (oldptr == 0) return asalloc(size);

    if ((char*)oldptr+size < heapend)
    {
        heapptr = (char*)oldptr + size;
        rv = oldptr;
    }
    else
        rv = 0;
#else
    rv = realloc(oldptr, size);
#endif

    if (rv == 0) as_abort(NOMEMEORY);
    return rv;
}

