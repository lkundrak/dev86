
#include <bios.h>
#include <fcntl.h>
#include <errno.h>

#include "io.h"
#include "rawio.h"

static int op_open();
static int op_close();
static int op_read();
static int op_write();
static long op_lseek();
static int fileops();

int
open(name, flags, mode)
char * name;
int flags, mode;
{
   __files = fileops;
   return (*__files)(CMD_OPEN, flags, name, mode);
}

static int
fileops(cmd, fd, buf, len)
int cmd, fd, len;
char * buf;
{ 
   switch(cmd)
   {
   case CMD_OPEN:  return op_open(buf, fd, len);
   case CMD_READ:  return op_read(fd, buf, len);
/*
   case CMD_WRITE: return op_write(fd, buf, len);
   case CMD_LSEEK: rv = op_lseek(fd, *(long*)buf, len);
                   (*(long*)buf) = rv;
		   if( rv == -1 ) return -1;
		   else return 0;
 */
   case CMD_CLOSE: return op_close(fd);
   }

   errno=EINVAL;
   return -1;
}

#define MAX_OPEN_FILES 5
ioblock _iob[MAX_OPEN_FILES];

static int
op_read(fd,buf,len)
int fd,len;
char * buf;
{
   ioblock* cur = &_iob[fd];
   int amount_read = 0;
   int amount_left_in_buffer;
   int amount_to_copy;

   if (fd < 0 || fd >= MAX_OPEN_FILES || _iob[fd].block_read == 0)
   {
      errno = EBADF;
      return -1;
   }

   while (len > 0)
   {
      /* pull in next block as required */
      if (cur->amount_left <= 0)
      {
         int read_len = cur->block_read(cur,
                                        cur->buffer,
                                        (long) cur->offset / sizeof(cur->buffer)
                                       );
#ifdef DEBUG
         fprintf(stderr, "br: returned %d\n", read_len);
#endif
         if (read_len <= 0)
            break;
         cur->amount_left = read_len;
      }
      if (cur->amount_left > len)
         amount_to_copy = len;
      else
         amount_to_copy = cur->amount_left;

#ifdef DEBUG
      fprintf(stderr, "r: len=%d, amount_left=%ld, offset=%ld, buf=%x\n",
              len, cur->amount_left, cur->offset,
              (int) cur->buffer[cur->offset % sizeof(cur->buffer)]);
#endif
      memcpy(buf,
             &cur->buffer[cur->offset % sizeof(cur->buffer)],
             amount_to_copy);
      amount_read += amount_to_copy;
      len -= amount_to_copy;
      cur->offset += amount_to_copy;
      buf += amount_to_copy;
      cur->amount_left -= amount_to_copy;
   }
   return amount_read;
}

/****************************************************************************/

static int
op_open(name, flags, mode)
char * name;
int flags, mode;
{
   int fd;
   ioblock* cur;

   /*
    * discover whether the iob has been initialised or not
    */
   if (_iob[0].flags == 0)
   {
      _iob[0].flags = O_RDONLY;
      _iob[1].flags = O_WRONLY;
      _iob[2].flags = O_WRONLY;
   }
   /*
    * discover next free iob
    */
   for (fd = 3; fd < MAX_OPEN_FILES; ++fd)
   {
      if (_iob[fd].block_read == NULL && _iob[fd].block_write == NULL)
         break;
   }
   if (fd >= MAX_OPEN_FILES)
   {
      errno = EMFILE;   /* too many open files */
      return -1;
   }

   /*
    * try and find the file
    */
   cur = &_iob[fd];
   if (fsdos_open_file(cur, name, flags, mode) >= 0)
   {
       cur->amount_left = 0;
       cur->offset = 0;
       return fd;
   }
   cur->block_read = NULL;      /* ensure that the file is closed */
   cur->block_write = NULL;
   errno = ENOENT;
   return -1;
}

/****************************************************************************/

static int
op_close(fd)
int fd;
{
   if (fd < 0 || fd >= MAX_OPEN_FILES || _iob[0].flags == 0)
   {
      errno = EBADF;
      return -1;
   }
   else
   {
       ioblock* cur = &_iob[fd];
       cur->close(cur);
       cur->block_read = NULL;
       cur->block_write = NULL;
       cur->close = NULL;
       cur->flags = 0;
       return 0;
   }
}

/****************************************************************************/

