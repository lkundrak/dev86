#ifndef __io_h__
#define __io_h__

typedef struct {
    /*
     * both block_read/block_write should be defined as
     * int x(ioblock* iob, char* buffer, int blockno)
     * and it reads/writes 1k blocks
     *
     * close should be defined as int x(ioblock* ioblock);
     */
    int (*block_read)();        /* read routine */
    int (*block_write)();       /* write routine - not supported yet*/
    int (*close)();             /* close routine */
    long offset;                /* current offset in file to read/write */
    int flags;
    long amount_left;           /* amount left in buffer */
    char buffer[1024];
    void* context;
} ioblock;

#endif
