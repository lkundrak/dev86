#ifndef __readfs_h__    /* multi-inclusion protection */
#define __readfs_h__

#ifdef __STDC__
#define P(x) x
#else
#define P(x) ()
#endif

#define rawio_read_sector(__sect, __buffer) rawio_read_lsector((unsigned long)__sect, __buffer)
int rawio_read_lsector P((long sector, char* buffer));

extern int rawio_disk_cyls;
extern int rawio_disk_heads;
extern int rawio_disk_spt;
#endif
