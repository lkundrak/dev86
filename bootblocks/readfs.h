
/* Functions for reading from one file at a time in the root directory
 * of a raw filesystem.
 */

#ifdef __STDC__
#define P(x) x
#else
#define P(x) ()
#endif

int open_file P((char * fname));
int rewind_file P((void));
int close_file P((void));
long file_length P((void));
int read_block P((char * buffer));

int tar_open_file P((char * fname));
int tar_rewind_file P((void));
int tar_close_file P((void));
long tar_file_length P((void));
int tar_read_block P((char * buffer));

int min_open_file P((char * fname));
int min_rewind_file P((void));
int min_close_file P((void));
long min_file_length P((void));
int min_read_block P((char * buffer));

int dos_open_file P((char * fname));
int dos_rewind_file P((void));
int dos_close_file P((void));
long dos_file_length P((void));
int dos_read_block P((char * buffer));

#define read_sector(__sect) read_lsector((unsigned long)(__sect))
char * read_lsector P((long sector));
void reset_disk P((void));

extern char * track_buffer;
extern int disk_drive;
extern int disk_cyls;
extern int disk_heads;
extern int disk_spt;
