
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

