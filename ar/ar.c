/* ar.c - Archive modify and extract.
   Copyright (C) 1988, 1990 Free Software Foundation, Inc.
   ar86 changes by Greg Haerr <greg@censoft.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


#include <stdio.h>
#include <stdlib.h>
#ifdef __STDC__
#include <unistd.h>
#include <string.h>
#include <utime.h>
#include <sys/time.h>
#endif
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "ar.h"
#include "rel_aout.h"

#if defined(__BCC__) || defined(__HP_cc)
#define HAVE_RENAME
#undef  HAVE_FSYNC
#define SHORT_FILENAME
#else
#define HAVE_FCHMOD
#define HAVE_RENAME
#undef  HAVE_FSYNC
#endif
#define HAVE_TRAILING_SLASH_IN_NAME

#ifdef	__GNUC__
# ifndef alloca
#  define	alloca	__builtin_alloca
# endif
#else
# if defined(sparc) || defined(HAVE_ALLOCA_H)
#  include <alloca.h>
# endif
#endif

#ifdef	USG
#define	bcopy(source, dest, size)	memcpy((dest), (source), (size))
#define	bcmp(a, b, size)		memcmp((a), (b), (size))
#define	bzero(s, size)			memset((s), 0, (size))
#endif

/* Locking is normally disabled because fcntl hangs on the Sun
   and it isn't supported properly across NFS anyway.  */

/* This structure is used internally to represent the info
   on a member of an archive.  This is to make it easier to change format.  */

struct member_desc
  {
    /* Name of member.  */
    char *name;

    /* The following fields are stored in the member header as decimal or octal
       numerals, but in this structure they are stored as machine numbers.  */
    int mode;		/* Protection mode from member header.  */
    long int date;	/* Last modify date as stored in member header.  */
    unsigned int size;	/* Bytes of member's data, from member header.  */
    int uid, gid;	/* UID and GID fields copied from member header.  */
    unsigned int offset;/* Offset in archive of the header of this member.  */
    unsigned int data_offset;/* Offset of first data byte of the member.  */

    /* The next field does not describe where the member was in the
       old archive, but rather where it will be in the modified archive.
       It is set up by write_archive.  */
    unsigned int new_offset;	/* Offset of this member in new archive */

    /* Symdef data for member.  Used only for files being inserted.  */
    struct symdef *symdefs;
    unsigned int nsymdefs;	/* Number of entries of symdef data.  */
    unsigned int string_size;	/* Size of strings needed by symdef data.  */
  };

/* Each symbol is recorded by something like this.  */

struct symdef
  {
    union
      {
	unsigned long int stringoffset;
	char *name;
      } s;
    unsigned long int offset;
  };

/* Nonzero means it's the name of an existing member;
   position new or moved files with respect to this one.  */

char *posname;

/* How to use `posname':
   POS_BEFORE means position before that member.
   POS_AFTER means position after that member.
   POS_DEFAULT if position by default; then `posname' should also be zero. */

enum { POS_DEFAULT, POS_BEFORE, POS_AFTER } postype;

/* Nonzero means describe each action performed.  */

int verbose;

/* Nonzero means don't warn about creating the archive file if necessary.  */

int silent_create;

/* Nonzero means don't replace existing members whose
   dates are more recent than the corresponding files.  */

int newer_only;

/* Nonzero means preserve dates of members when extracting them.  */

int preserve_dates;

/* Operation to be performed.  */

#define DELETE 1
#define REPLACE 2
#define PRINT_TABLE 3
#define PRINT_FILES 4
#define EXTRACT 5
#define MOVE 6
#define QUICK_APPEND 7

int operation;

/* Name of archive file.  */

char *archive;

/* Descriptor on which we have locked the original archive file,
   or -1 if this has not been done.  */

int lock_indesc;

/* Pointer to tail of `argv', at first subfile name argument,
 or zero if no such were specified.  */

char **files;

/* Nonzero means write a __.SYMDEF member into the modified archive.  */
int symdef_flag;

/* Nonzero means __.SYMDEF member exists in old archive.  */
int symdef_exists;

/* Nonzero means don't update __.SYMDEF unless the flag was given.  */
int ignore_symdef;

/* Total number of symdef entries we will have. */
unsigned long int nsymdefs;

/* Symdef data from old archive (set up only if we need it) */
struct symdef *old_symdefs;

/* Number of symdefs in remaining in old_symdefs.  */
unsigned int num_old_symdefs;

/* Number of symdefs old_symdefs had when it was read in.  */
unsigned long int original_num_symdefs;

/* String table from old __.SYMDEF member.  */
char *old_strings;

/* Size of old_strings */
unsigned long int old_strings_size;

/* String table to be written into __.SYMDEF member.  */
char *new_strings;

/* Size of new_strings */
unsigned long int new_strings_size;

/* An archive map is a chain of these structures.
  Each structure describes one member of the archive.
  The chain is in the same order as the members are.  */

struct mapelt
{
  struct member_desc info;
  struct mapelt *next;
};

struct mapelt *maplast;

/* If nonzero, this is the map-element for the __.SYMDEF member
   and we should update the time of that member just before finishing.  */
struct mapelt *symdef_mapelt;

/* Header that we wrote for the __.SYMDEF member.  */
struct ar_hdr symdef_header;

/* Name this program was run with. */
char *program_name;

#ifndef __STDC__
char *xmalloc (), *xrealloc ();
void free ();

void add_to_map (), delete_from_map ();
int insert_in_map ();
void print_descr ();
char *concat ();
void scan ();
void extract_members ();
void extract_member ();
void print_contents ();
void write_symdef_member ();
void read_old_symdefs ();
void two_operations ();
void usage (), fatal (), error (), error3(), error_with_file ();
void perror_with_name (), pfatal_with_name ();
void write_archive ();
void touch_symdef_member ();
void update_symdefs ();
void delete_members (), move_members (), replace_members ();
void quick_append ();
char *basename ();
void print_modes ();
char *make_tempname ();
void copy_out_member ();
#define const
#else
/* Grrr. */
extern void error (char * s1, char * s2);
extern void error3 (char * s1, char * s2, char * s3);

extern void fatal (char * s1, char * s2);
extern void extract_members (void (*function) (struct member_desc member, FILE *istream));
extern void scan (void (*function) (struct member_desc member, FILE *istream), int crflag);
extern char *basename (char *path);
extern char *concat (const char *s1, const char *s2, const char *s3);
extern char *make_tempname (char *name);
extern char *xmalloc (unsigned int size);
extern char *xrealloc (char *ptr, unsigned int size);
extern int filter_symbols (struct nlist *syms, unsigned int symcount);
extern int insert_in_map (char *name, struct mapelt *map, struct mapelt *after);
extern int main (int argc, char **argv);
extern int move_in_map (char *name, struct mapelt *map, struct mapelt *after);
extern int read_header_info (struct mapelt *mapelt, int desc, long int offset, long int *syms_offset, unsigned int *syms_size, long int *strs_offset, unsigned int *strs_size);
extern struct mapelt *find_mapelt (struct mapelt *map, char *name);
extern struct mapelt *find_mapelt_noerror (struct mapelt *map, register char *name);
extern struct mapelt *last_mapelt (struct mapelt *map);
extern struct mapelt *make_map (int nonexistent_ok);
extern struct mapelt *prev_mapelt (struct mapelt *map, struct mapelt *elt);
extern void add_to_map (struct member_desc member, FILE * istream);
extern void close_archive (void);
extern void copy_out_member (struct mapelt *mapelt, int archive_indesc, int outdesc, char *outname);
extern void delete_from_map (char *name, struct mapelt *map);
extern void delete_members (void);
extern void error_with_file (char *string, struct mapelt *mapelt);
extern void extract_member (struct member_desc member, FILE *istream);
extern void header_from_map (struct ar_hdr *header, struct mapelt *mapelt);
extern void lock_for_update (void);
extern void make_new_symdefs (struct mapelt *mapelt, int archive_indesc);
extern void move_members (void);
extern void mywrite (int desc, void *buf, int bytes, char *file);
extern void perror_with_name (char *name);
extern void pfatal_with_name (char *name);
extern void print_contents (struct member_desc member, FILE *istream);
extern void print_descr (struct member_desc member, FILE * instream);
extern void print_modes (int modes);
extern void quick_append (void);
extern void read_old_symdefs (struct mapelt *map, int archive_indesc);
extern void replace_members (void);
extern void touch_symdef_member (int outdesc, char *outname);
extern void two_operations (void);
extern void update_symdefs (struct mapelt *map, int archive_indesc);
extern void usage (char *s1, int val);
extern void write_archive (struct mapelt *map, int appendflag);
extern void write_symdef_member (struct mapelt *mapelt, struct mapelt *map, int outdesc, char *outname);
#endif

/* Output BYTES of data at BUF to the descriptor DESC.
   FILE is the name of the file (for error messages).  */

void
mywrite (desc, pbuf, bytes, file)
     int desc;
     void *pbuf;
     int bytes;
     char *file;
{
  register int val;
  register char * buf = pbuf;

  while (bytes > 0)
    {
      val = write (desc, buf, bytes);
      if (val <= 0)
	perror_with_name (file);
      buf += val;
      bytes -= val;
    }
}

int
main (argc, argv)
     int argc;
     char **argv;
{
  int i;

  operation = 0;
  verbose = 0;
  newer_only = 0;
  silent_create = 0;
  posname = 0;
  postype = POS_DEFAULT;
  preserve_dates = 0;
  symdef_flag = 0;
  symdef_exists = 0;
  ignore_symdef = 0;
  symdef_mapelt = 0;
  files = 0;
  lock_indesc = -1;

  program_name = argv[0];

  if (argc < 2)
    usage ("too few command arguments", 0);

  {
    char *key = argv[1];
    char *p = key;
    char c;

    if (*p == '-')
      p++;
   while ((c = *p++))
      {
	switch (c)
	  {
	  case 'a':
	    postype = POS_AFTER;
	    break;

	  case 'b':
	    postype = POS_BEFORE;
	    break;

	  case 'c':
	    silent_create = 1;
	    break;

	  case 'd':
	    if (operation)
	      two_operations ();

	    operation = DELETE;
	    break;

	  case 'i':
	    postype = POS_BEFORE;
	    break;

	  case 'l':
	    break;

	  case 'm':
	    if (operation)
	      two_operations ();
	    operation = MOVE;
	    break;

	  case 'o':
	    preserve_dates = 1;
	    break;

	  case 'p':
	    if (operation)
	      two_operations ();
	    operation = PRINT_FILES;
	    break;

	  case 'q':
	    if (operation)
	      two_operations ();
	    operation = QUICK_APPEND;
	    break;

	  case 'r':
	    if (operation)
	      two_operations ();
	    operation = REPLACE;
	    break;

	  case 's':
	    symdef_flag = 1;
	    break;

	  case 't':
	    if (operation)
	      two_operations ();
	    operation = PRINT_TABLE;
	    break;

	  case 'u':
	    operation = REPLACE;
	    newer_only = 1;
	    break;

	  case 'v':
	    verbose = 1;
	    break;

	  case 'x':
	    if (operation)
	      two_operations ();
	    operation = EXTRACT;
	    break;
	  }
      }
  
  }

  if (operation == 0 && symdef_flag)
    operation = REPLACE;

  if (operation == 0)
    usage ("no operation specified", 0);

  i = 2;

  if (postype != POS_DEFAULT)
  {
    if (i < argc)
      posname = argv[i++];
    else
      usage ("no position operand", 0);
  }

  if (i >= argc)
    usage ("no archive specified", 0);
  archive = argv[i++];

  if (i < argc)
    {
      files = &argv[i];
      while (i < argc)
	if (!strcmp (argv[i++], "__.SYMDEF"))
	  {
	    ignore_symdef = 1;
	    break;
	  }
    }

  switch (operation)
    {
    case EXTRACT:
	extract_members (extract_member);
	break;

    case PRINT_TABLE:
	extract_members (print_descr);
	break;

    case PRINT_FILES:
	extract_members (print_contents);
	break;

    case DELETE:
	if (files != 0)
	  delete_members ();
	break;

    case MOVE:
	if (files != 0)
	  move_members ();
	break;

    case REPLACE:
	if (files != 0 || symdef_flag)
	  replace_members ();
	break;

    case QUICK_APPEND:
	if (files != 0)
	  quick_append ();
	break;

    default:
	usage ("invalid operation %d", operation);
    }

  exit (0);
}

void
two_operations ()
{
  usage ("two different operation switches specified", 0);
}

void
scan (function, crflag)
#ifdef __STDC__
     void (*function) (struct member_desc member, FILE *istream);
#else
     void (*function) ();
#endif
     int crflag;
{
  FILE *arcstream = fopen (archive, "r");

  if (arcstream == 0 && crflag)
    /* Creation-warning, if desired, will happen later.  */
    return;

  if (arcstream == 0)
    {
      perror_with_name (archive);
      exit (1);
    }
  {
    char buf[SARMAG];
    int nread = fread (buf, 1, SARMAG, arcstream);
    if (nread != SARMAG || bcmp (buf, ARMAG, SARMAG))
      fatal ("file %s not a valid archive", archive);
  }

  /* Now find the members one by one.  */
  {
    int member_offset = SARMAG;
    while (1)
      {
	int nread;
	struct ar_hdr member_header;
	struct member_desc member_desc;
	char name [1 + sizeof member_header.ar_name];

	if (fseek (arcstream, member_offset, 0) < 0)
	  perror_with_name (archive);

	nread = fread (&member_header, 1, sizeof (struct ar_hdr), arcstream);
	if (nread == 0)
	  /* No data left means end of file; that is OK.  */
	  break;

	if (nread != sizeof (member_header)
	    || bcmp (member_header.ar_fmag, ARFMAG, 2))
	  fatal ("file %s not a valid archive", archive);
	bcopy (member_header.ar_name, name, sizeof member_header.ar_name);
	{
	  char *p = name + sizeof member_header.ar_name;
	  *p = '\0';
	  while (p > name && *--p == ' ')
	    *p = '\0';
#if defined(USG) || defined(HAVE_TRAILING_SLASH_IN_NAME)
	  if (*p == '/')	/* Since V.2, names are terminated with '/' */
	    *p = '\0';
#endif
        }
	member_desc.name = name;
	sscanf (member_header.ar_mode, "%o", &member_desc.mode);
	member_desc.date = atoi (member_header.ar_date);
	member_desc.size = atoi (member_header.ar_size);
	member_desc.uid = atoi (member_header.ar_uid);
	member_desc.gid = atoi (member_header.ar_gid);
	member_desc.offset = member_offset;
	member_desc.data_offset = member_offset + sizeof (member_header);

	member_desc.new_offset = 0;
	member_desc.symdefs = 0;
	member_desc.nsymdefs = 0;
	member_desc.string_size = 0;

	if (!ignore_symdef && !strcmp (name, "__.SYMDEF"))
	  symdef_exists = 1;

	function (member_desc, arcstream);

	member_offset += sizeof (member_header) + member_desc.size;
	if (member_offset & 1)
	  ++member_offset;
      }
  }

  
  fclose (arcstream);
}

void
print_descr (member, instream)
     struct member_desc member;
     FILE * instream;
{
  char *timestring;
  if (!verbose)
    {
      puts (member.name);
      return;
    }
  print_modes (member.mode);
  timestring = ctime (&member.date);
  printf (" %2d/%2d %6d %12.12s %4.4s %s\n",
	  member.uid, member.gid,
	  member.size, timestring + 4, timestring + 20,
	  member.name);
}

void
print_modes (modes)
     int modes;
{
  putchar (modes & 0400 ? 'r' : '-');
  putchar (modes & 0200 ? 'w' : '-');
  putchar (modes & 0100 ? 'x' : '-');
  putchar (modes & 040 ? 'r' : '-');
  putchar (modes & 020 ? 'w' : '-');
  putchar (modes & 010 ? 'x' : '-');
  putchar (modes & 04 ? 'r' : '-');
  putchar (modes & 02 ? 'w' : '-');
  putchar (modes & 01 ? 'x' : '-');
}

#define BUFSIZE 1024

void
extract_member (member, istream)
     struct member_desc member;
     FILE *istream;
{
  int ncopied = 0;
  FILE *ostream;

  fseek (istream, member.data_offset, 0);
  ostream = fopen (member.name, "w");
  if (!ostream)
    {
      perror_with_name (member.name);
      return;
    }

  if (verbose)
    printf ("x - %s\n", member.name);

  while (ncopied < member.size)
    {
      char buf [BUFSIZE];
      int tocopy = member.size - ncopied;
      int nread;
      if (tocopy > BUFSIZE) tocopy = BUFSIZE;
      nread = fread (buf, 1, tocopy, istream);
      if (nread != tocopy)
	fatal ("error reading archive %s", archive);
      fwrite (buf, 1, nread, ostream);
      ncopied += tocopy;
    }

#ifdef HAVE_FCHMOD
  fchmod (fileno (ostream), member.mode);
#else
  chmod (member.name, member.mode);
#endif
  if (ferror (ostream) || fclose (ostream) != 0)
    error ("%s: I/O error", member.name);

  if (preserve_dates)
    {
#if defined(USG) || defined(__BCC__)
      long tv[2];
      tv[0] = member.date;
      tv[1] = member.date;
      utime (member.name, tv);
#else
      struct timeval tv[2];
      tv[0].tv_sec = member.date;
      tv[0].tv_usec = 0;
      tv[1].tv_sec = member.date;
      tv[1].tv_usec = 0;
      utimes (member.name, tv);
#endif
    }
}

void
print_contents (member, istream)
     struct member_desc member;
     FILE *istream;
{
  int ncopied = 0;

  fseek (istream, member.data_offset, 0);

  if (verbose)
  printf ("\n<member %s>\n\n", member.name);

  while (ncopied < member.size)
    {
      char buf [BUFSIZE];
      int tocopy = member.size - ncopied;
      int nread;
      if (tocopy > BUFSIZE) tocopy = BUFSIZE;
      nread = fread (buf, 1, tocopy, istream);
      if (nread != tocopy)
	fatal ("file %s not a valid archive", archive);
      fwrite (buf, 1, nread, stdout);
      ncopied += tocopy;
    }
}

/* Make a map of the existing members of the archive: their names,
 positions and sizes.  */

/* If `nonexistent_ok' is nonzero,
 just return 0 for an archive that does not exist.
 This will cause the ordinary supersede procedure to
 create a new archive.  */

struct mapelt *
make_map (nonexistent_ok)
     int nonexistent_ok;
{
  struct mapelt mapstart;
  mapstart.next = 0;
  maplast = &mapstart;
  scan (add_to_map, nonexistent_ok);
  return mapstart.next;
}

void
add_to_map (member, istream)
     struct member_desc member;
     FILE * istream;
{
  struct mapelt *mapelt = (struct mapelt *) xmalloc (sizeof (struct mapelt));
  mapelt->info = member;
  mapelt->info.name = concat (mapelt->info.name, "", "");
  maplast->next = mapelt;
  mapelt->next = 0;
  maplast = mapelt;
}

/* Return the last element of the specified map.  */

struct mapelt *
last_mapelt (map)
     struct mapelt *map;
{
  struct mapelt *tail = map;
  while (tail->next) tail = tail->next;
  return tail;
}

/* Return the element of the specified map which precedes elt.  */

struct mapelt *
prev_mapelt (map, elt)
     struct mapelt *map, *elt;
{
  struct mapelt *tail = map;
  while (tail->next && tail->next != elt)
    tail = tail->next;
  if (tail->next) return tail;
  return 0;
}

/* Return the element of the specified map which has the specified name.  */

struct mapelt *
find_mapelt_noerror (map, name)
     struct mapelt *map;
     register char *name;
{
  register struct mapelt *tail;
  unsigned int len;
  int dot_o;

  name = basename (name);
  len = strlen (name);
  dot_o = name[len - 2] == '.' && name[len - 1] == 'o';

  for (tail = map; tail != 0; tail = tail->next)
    {
      if (tail->info.name == 0)
	continue;
      if (!strncmp (tail->info.name, name, 13))
	{
	  unsigned int eltlen = strlen (tail->info.name);
	  if (len <= 13 || eltlen <= 13)
	    return tail;
	  else
	    {
	      char *p = tail->info.name + 13;
	      if (dot_o && p[0] == '.' && p[1] == 'o' && p[2] == '\0')
		return tail;
	      else if (!strncmp (p, name + 13,
				 (len > eltlen ? len : eltlen) - 13))
		return tail;
	    }
	}
    }

  return 0;
}

struct mapelt *
find_mapelt (map, name)
     struct mapelt *map;
     char *name;
{
  register struct mapelt *found = find_mapelt_noerror (map, name);
  if (found == 0)
    error ("no member named `%s'", name);
  return found;
}

/* Before looking at the archive, if we are going to update it
   based on looking at its current contents, make an exclusive lock on it.
   The lock is released when `write_archive' is called.  */

void
lock_for_update ()
{
  /* Open the existing archive file; if that fails, create an empty one.  */

  lock_indesc = open (archive, O_RDWR, 0);

  if (lock_indesc < 0)
    {
      int outdesc;

      if (!silent_create)
	printf ("Creating archive file `%s'\n", archive);
      outdesc = open (archive, O_WRONLY | O_APPEND | O_CREAT, 0666);
      if (outdesc < 0)
	pfatal_with_name (archive);
      mywrite (outdesc, ARMAG, SARMAG, archive);
      close (outdesc);

      /* Now we had better be able to open for update!  */

      lock_indesc = open (archive, O_RDWR, 0);
      if (lock_indesc < 0)
	{
	  unlink (archive);
	  pfatal_with_name (archive);
	}
    }

#ifdef LOCKS
  /* Lock the old file so that it won't be updated by two programs at once.
     This uses the fcntl locking facility found on Sun systems
     which is also in POSIX.  (Perhaps it comes from sysV.)

     Note that merely reading an archive does not require a lock,
     because we use `rename' to update the whole file atomically.  */

  {
    struct flock lock;

    lock.l_type = F_WRLCK;
    lock.l_whence = 0;
    lock.l_start = 0;
    lock.l_len = 0;

    while (1)
      {
	int value = fcntl (lock_indesc, F_SETLKW, &lock);
	if (value >= 0)
	  break;
	else if (errno == EINTR)
	  continue;
	else
	  pfatal_with_name ("locking archive");
      }
  }
#endif
}

/* Unlock archive and close the file descriptor.  */

void
close_archive ()
{
#ifdef LOCKS
  {
    struct flock lock;

    /* Unlock the old archive.  */

    lock.l_type = F_UNLCK;
    lock.l_whence = 0;
    lock.l_start = 0;
    lock.l_len = 0;

    fcntl (lock_indesc, F_SETLK, &lock);
  }
#endif

  /* Close the archive.  If we renamed a new one, the old one disappears.  */
  close (lock_indesc);
}

/* Write a new archive file from a given map.  */
/* When a map is used as the pattern for a new archive,
 each element represents one member to put in it, and
 the order of elements controls the order of writing.

 Ordinarily, the element describes a member of the old
 archive, to be copied into the new one.

 If the `offset' field of the element's info is 0,
 then the element describes a file to be copied into the
 new archive.  The `name' field is the file's name.

 If the `name' field of an element is 0, the element is ignored.
 This makes it easy to specify deletion of archive members.

 Every operation that will eventually call `write_archive'
 should call `lock_for_update' before beginning
 to do any I/O on the archive file.
*/

void
write_archive (map, appendflag)
     struct mapelt *map;
     int appendflag;
{
  char *tempname = make_tempname (archive);
  int indesc = lock_indesc;
  int outdesc;
  char *outname;
  struct mapelt *tail;

  /* Now open the output.  */

  if (!appendflag)
    {
      /* Updating an existing archive normally.
	 Write output as TEMPNAME and rename at the end.
	 There can never be two invocations trying to do this at once,
	 because of the lock made on the old archive file.  */

      outdesc = open (tempname, O_WRONLY | O_CREAT, 0666);
      if (outdesc < 0)
	pfatal_with_name (tempname);
      outname = tempname;
      mywrite (outdesc, ARMAG, SARMAG, outname);
    }
  else
    {
      /* Fast-append to existing archive.  */

      outdesc = open (archive, O_WRONLY | O_APPEND, 0);
      if (outdesc < 0)
	pfatal_with_name (archive);
      outname = archive;
    }

  /* If archive has or should have a __.SYMDEF member,
     compute the contents for it.  */

  if (symdef_flag || symdef_exists)
    {
      if (symdef_exists)
	read_old_symdefs (map, indesc);
      else
	{
	  struct mapelt *this = (struct mapelt *)
	    xmalloc (sizeof (struct mapelt));
	  this->info.name = "__.SYMDEF";
	  this->info.offset = SARMAG;
	  this->info.data_offset = SARMAG + sizeof (struct ar_hdr);
	  this->info.new_offset = 0;
	  this->info.date = 0;
	  this->info.size = 0;
	  this->info.uid = 0;
	  this->info.gid = 0;
	  this->info.mode = 0666;
	  this->info.symdefs = 0;
	  this->info.nsymdefs = 0;
	  this->info.string_size = 0;
	  this->next = map;
	  map = this;
	  original_num_symdefs = 0;
	  old_strings_size = 0;
	}

      update_symdefs (map, indesc);
    }

  /* Copy the members into the output, either from the old archive
     or from specified files.  */

  for (tail = map; tail != 0; tail = tail->next)
    {
      if ((symdef_flag || symdef_exists) && tail->info.name
	  && !strcmp (tail->info.name, "__.SYMDEF")
#if 0
	  && tail->info.date==0
#endif
	  )
	write_symdef_member (tail, map, outdesc, outname);
      else
	copy_out_member (tail, indesc, outdesc, outname);
    }

  if (symdef_mapelt != 0)
    {
      /* Check for members whose data offsets weren't
	 known when the symdef member was first written.  */
      int doneany = 0;
      for (tail = map; tail != 0; tail = tail->next)
	if (tail->info.offset == 0)
	  {
	    /* Fix up the symdefs.  */
	    register unsigned int i;
	    for (i = 0; i < tail->info.nsymdefs; ++i)
	      tail->info.symdefs[i].offset = tail->info.new_offset;
	    doneany = 1;
	  }
      if (doneany)
	{
	  /* Some files had bad symdefs; rewrite the symdef member.  */
	  lseek (outdesc, symdef_mapelt->info.offset, 0);
	  write_symdef_member (symdef_mapelt, map, outdesc, outname);
	}
    }

  /* Mark the __.SYMDEF member as up to date.  */

  if (symdef_mapelt != 0)
    touch_symdef_member (outdesc, outname);

  /* Install the new output under the intended name.  */

#ifdef HAVE_FSYNC
  fsync (outdesc);
#endif
  close (outdesc);

  if (!appendflag)
    if (rename (tempname, archive))
      pfatal_with_name (tempname);

  close_archive ();
}

void
header_from_map (header, mapelt)
     struct ar_hdr *header;
     struct mapelt *mapelt;
{
  unsigned int namelen;
  char *p;

  /* Zero the header, then store in the data as text.  */
  bzero ((char *) header, sizeof (*header));

  p = basename (mapelt->info.name);
  strncpy (header->ar_name, p, sizeof (header->ar_name));
  namelen = strlen (p);
  if (namelen >= sizeof (header->ar_name))
    {
      if (mapelt->info.name[namelen - 2] == '.' &&
	  mapelt->info.name[namelen - 1] == 'o')
	{
	  header->ar_name[sizeof (header->ar_name) - 3] = '.';
	  header->ar_name[sizeof (header->ar_name) - 2] = 'o';
	}
      header->ar_name[sizeof (header->ar_name) - 1] = '\0';
      error3 ("member name `%s' truncated to `%s'",
	     mapelt->info.name, header->ar_name);
    }
#if defined(USG) || defined(HAVE_TRAILING_SLASH_IN_NAME)
  {
    /* System V tacks a trailing '/' onto the end of the name */
    char *p = header->ar_name;
    char *end = p + sizeof (header->ar_name);

    while (p < end && *p)
      p++;
    *p = '/';
  }
#endif

  sprintf (header->ar_date, "%ld", mapelt->info.date);
  sprintf (header->ar_size, "%d", mapelt->info.size);
  sprintf (header->ar_uid, "%d", mapelt->info.uid);
  sprintf (header->ar_gid, "%d", mapelt->info.gid);
  sprintf (header->ar_mode, "%o", mapelt->info.mode);
  strncpy (header->ar_fmag, ARFMAG, sizeof (header->ar_fmag));

  /* Change all remaining nulls in the header into spaces.  */
  {
    char *end = (char *) &header[1];
    register char *p;
    for (p = (char *) header; p < end; ++p)
      if (*p == '\0')
	*p = ' ';
  }
}

/* gets just the file part of name */

char *
basename (path)
     char *path;
{
  char *save, *start;
  for (start = save = path; *path; path++)
    if (*path == '/')
      save = path + 1;

  if (save != start)
    return save;
  else
    return start;
}

/* writes to file open on OUTDESC with name OUTNAME.  */
void
copy_out_member (mapelt, archive_indesc, outdesc, outname)
     struct mapelt *mapelt;
     int archive_indesc;
     int outdesc;
     char *outname;
{
  struct ar_hdr header;
  int indesc;

  if (mapelt->info.name == 0)
    /* This element was cancelled.  */
    return;

  header_from_map (&header, mapelt);

  if (mapelt->info.offset != 0)
    {
      indesc = archive_indesc;
      lseek (indesc, mapelt->info.data_offset, 0);
    }
  else
    {
      indesc = open (mapelt->info.name, 0, 0);
      if (indesc < 0)
	{
	  perror_with_name (mapelt->info.name);
	  return;
	}
    }

  mywrite (outdesc, &header, sizeof (header), outname);

  if (mapelt->info.data_offset == 0)
    mapelt->info.data_offset = lseek (outdesc, 0L, 1);

  {
    char buf[BUFSIZE];
    int tocopy = mapelt->info.size;
    while (tocopy > 0)
      {
	int thistime = tocopy;
	if (thistime > BUFSIZE) thistime = BUFSIZE;
        read (indesc, buf, thistime);
	mywrite (outdesc, buf, thistime, outname);
	tocopy -= thistime;
      }
  }

  if (indesc != archive_indesc)
    close (indesc);

  if (mapelt->info.size & 1)
    mywrite (outdesc, "\n", 1, outname);
}

/* Update the time of the __.SYMDEF member; done when we updated
   that member, just before we close the new archive file.
   It is open on OUTDESC and its name is OUTNAME.  */

void
touch_symdef_member (outdesc, outname)
     int outdesc;
     char *outname;
{
  struct stat statbuf;
  int i;

  /* See what mtime the archive file has as a result of our writing it.  */
  fstat (outdesc, &statbuf);

  /* Advance member's time to that time.  */
  bzero (symdef_header.ar_date, sizeof symdef_header.ar_date);
  sprintf (symdef_header.ar_date, "%ld", statbuf.st_mtime);
  for (i = 0; i < sizeof symdef_header.ar_date; i++)
    if (symdef_header.ar_date[i] == 0)
      symdef_header.ar_date[i] = ' ';

  /* Write back this member's header with the new time.  */
  if (lseek (outdesc, symdef_mapelt->info.new_offset, 0) >= 0)
    mywrite (outdesc, &symdef_header, sizeof symdef_header, outname);
}

char *
make_tempname (name)
     char *name;
{
#if defined(USG) || defined(SHORT_FILENAME)
  /* sigh. 14 character filenames are *wonderful*, just *wonderful*. */
  char *p = basename (name);
  char *q, *r;

  if (p != name)
    {
      q = concat (name, "", "");	/* get a fresh copy */
      r = basename (q);	/* r points just after last '/' */
      *--r = '\0';
      return concat (q, "/t_", p);
    }
  else if (strlen (name) >= 14)
    return concat ("t_", name, "");
  else
#endif
  return concat (name, "", "_supersede");
}

void
delete_members ()
{
  struct mapelt *map = make_map (0);
  struct mapelt mapstart;
  char **p;

  mapstart.info.name = 0;
  mapstart.next = map;
  map = &mapstart;

  lock_for_update ();

  if (files)
    for (p = files; *p; p++)
      {
	/* If user says to delete the __.SYMDEF member,
	   don't make a new one to replace it.  */
	if (!strcmp (*p, "__.SYMDEF"))
	  symdef_exists = 0;
	delete_from_map (*p, map);
      }

  write_archive (map->next, 0);
}

void
delete_from_map (name, map)
     char *name;
     struct mapelt *map;
{
  struct mapelt *this = find_mapelt (map, name);

  if (!this) return;
  this->info.name = 0;
  if (verbose)
    printf ("d - %s\n", name);
}

void
move_members ()
{
  struct mapelt *map = make_map (0);
  char **p;
  struct mapelt *after_mapelt = 0;
  struct mapelt mapstart;
  struct mapelt *change_map;

  mapstart.info.name = 0;
  mapstart.next = map;
  change_map = &mapstart;

  lock_for_update ();

  switch (postype)
    {
    case POS_DEFAULT:
      after_mapelt = last_mapelt (change_map);
      break;

    case POS_AFTER:
      after_mapelt = find_mapelt (map, posname);
      break;

    case POS_BEFORE:
      after_mapelt = prev_mapelt (change_map, find_mapelt (map, posname));
    }

  /* Failure to find specified "before" or "after" member
     is a fatal error; message has already been printed.  */

  if (!after_mapelt) exit (1);

  if (files)
    for (p = files; *p; p++)
      {
	if (move_in_map (*p, change_map, after_mapelt))
	  after_mapelt = after_mapelt->next;
      }

  write_archive (map, 0);
}

int
move_in_map (name, map, after)
     char *name;
     struct mapelt *map, *after;
{
  struct mapelt *this = find_mapelt (map, name);
  struct mapelt *prev;

  if (!this) return 0;
  prev = prev_mapelt (map, this);
  prev->next = this->next;
  this->next = after->next;
  after->next = this;
  return 1;
}

/* Insert files into the archive.  */

void
replace_members ()
{
  struct mapelt *map = make_map (1);
  struct mapelt mapstart;
  struct mapelt *after_mapelt = 0;
  struct mapelt *change_map;
  char **p;
  int changed;

  mapstart.info.name = 0;
  mapstart.next = map;
  change_map = &mapstart;

  lock_for_update ();

  switch (postype)
    {
    case POS_DEFAULT:
      after_mapelt = last_mapelt (change_map);
      break;

    case POS_AFTER:
      after_mapelt = find_mapelt (map, posname);
      break;

    case POS_BEFORE:
      after_mapelt = prev_mapelt (change_map, find_mapelt (map, posname));
    }

  /* Failure to find specified "before" or "after" member
     is a fatal error; the message has already been printed.  */
  if (after_mapelt == 0)
    exit (1);

  changed = 0;
  if (files != 0)
    for (p = files; *p != 0; ++p)
      if (insert_in_map (*p, change_map, after_mapelt))
	{
	  after_mapelt = after_mapelt->next;
	  changed = 1;
	}

  change_map = change_map->next;
  if (!changed && (!symdef_flag || symdef_exists))
    /* Nothing changed.  */
    close_archive ();
  else
    write_archive (change_map, 0);
}

/* Handle the "quick insert" operation.  */

void
quick_append ()
{
  struct mapelt *map;
  struct mapelt *after;
  struct mapelt mapstart;
  char **p;

  mapstart.info.name = 0;
  mapstart.next = 0;
  map = &mapstart;
  after = map;

  lock_for_update ();

  /* Insert the specified files into the "map",
     but is a map of the inserted files only,
     and starts out empty.  */
  if (files)
    for (p = files; *p; p++)
      {
	if (insert_in_map (*p, map, after))
	  after = after->next;
      }

  /* Append these files to the end of the existing archive file.  */

  write_archive (map->next, 1);
}

/* Insert an entry for name NAME into the map MAP after the map entry AFTER.
   Delete an old entry for NAME.
   MAP is assumed to start with a dummy entry, which facilitates
   insertion at the beginning of the list.
   Return 1 if successful, 0 if did nothing because file NAME doesn't
   exist or (optionally) is older.  */

int
insert_in_map (name, map, after)
     char *name;
     struct mapelt *map, *after;
{
  struct mapelt *old = find_mapelt_noerror (map, name);
  struct mapelt *this;
  struct stat status;

  if (stat (name, &status))
    {
      perror_with_name (name);
      return 0;
    }
  if (old && newer_only && status.st_mtime <= old->info.date)
    return 0;
  if (old)
    /* Delete the old one.  */
    old->info.name = 0;
  this = (struct mapelt *) xmalloc (sizeof (struct mapelt));
  this->info.name = name;
  this->info.offset = 0;
  this->info.data_offset = 0;
  this->info.date = status.st_mtime;
  this->info.size = status.st_size;
  this->info.uid = status.st_uid;
  this->info.gid = status.st_gid;
  this->info.mode = status.st_mode;
  /* Always place a __.SYMDEF member first in the archive, regardless
     of any position specifications.  */
  if (! strcmp (name, "__.SYMDEF"))
    this->next = map->next, map->next = this;
  else
    this->next = after->next, after->next = this;

  if (verbose)
    printf ("%c - %s\n", old == 0 ? 'a' : 'r', this->info.name);

  return 1;
}

/* Apply a function to each of the specified members.
*/

void
extract_members (function)
#ifdef __STDC__
     void (*function) (struct member_desc member, FILE *istream);
#else
     void (*function) ();
#endif
{
  struct mapelt *map;
  FILE *arcstream;
  char **p;

  if (!files)
    {
      /* Handle case where we want to operate on every member.
	 No need to make a map and search it for this.  */
      scan (function, 0);
      return;
    }

  arcstream = fopen (archive, "r");
  if (!arcstream)
    fatal ("failure opening archive %s for the second time", archive);
  map = make_map (0);

  for (p = files; *p; p++)
    {
      struct mapelt *this = find_mapelt (map, *p);
      if (!this) continue;
      function (this->info, arcstream);
    }

  fclose (arcstream);
}


/* Write the __.SYMDEF member from data in core.  OUTDESC and OUTNAME
   are descriptor and name of file to write to.  */

void
write_symdef_member (mapelt, map, outdesc, outname)
     struct mapelt *mapelt;
     struct mapelt *map;
     int outdesc;
     char *outname;
{
  struct ar_hdr header;
  struct mapelt *mapptr;
  unsigned long int symdefs_size;

  if (mapelt->info.name == 0)
    /* This element was cancelled.  */
    return;

  header_from_map (&header, mapelt);

  bcopy (&header, &symdef_header, sizeof header);

  mywrite (outdesc, &header, sizeof (header), outname);

  /* Write the number of symdefs.  */
  symdefs_size = nsymdefs * sizeof (struct symdef);
#ifdef HOST_TARGET_ENDIANESS_DIFFERS
  md_number_to_chars (symdefs_size, sizeof symdefs_size,
		      (unsigned char *) &symdefs_size);
#endif
  mywrite (outdesc, &symdefs_size, sizeof symdefs_size, outname);

  /* Write symdefs surviving from old archive.  */
#ifdef HOST_TARGET_ENDIANESS_DIFFERS
  md_symdefs_to_chars ((struct md_symdef *) old_symdefs, num_old_symdefs,
		       (unsigned char *) old_symdefs);
#endif
  mywrite (outdesc, old_symdefs, num_old_symdefs * sizeof (struct symdef),
	   outname);
#ifdef HOST_TARGET_ENDIANESS_DIFFERS
  md_chars_to_symdefs ((unsigned char *) old_symdefs,
		       (struct md_symdef *) old_symdefs, num_old_symdefs);
#endif

  /* Write symdefs for new members.  */
  for (mapptr = map; mapptr != 0; mapptr = mapptr->next)
    if (mapptr->info.nsymdefs != 0)
      {
#ifdef HOST_TARGET_ENDIANESS_DIFFERS
	md_symdefs_to_chars ((struct md_symdef *) mapptr->info.symdefs,
			     mapptr->info.nsymdefs,
			     (unsigned char *) mapptr->info.symdefs);
#endif
	write (outdesc, mapptr->info.symdefs,
	       mapptr->info.nsymdefs * sizeof (struct symdef));
#ifdef HOST_TARGET_ENDIANESS_DIFFERS
	md_chars_to_symdefs ((unsigned char *) mapptr->info.symdefs,
			     (struct md_symdef *) mapptr->info.symdefs,
			     mapptr->info.nsymdefs);
#endif
      }

  /* Write the string table size.  */
#ifdef HOST_TARGET_ENDIANESS_DIFFERS
  md_number_to_chars (new_strings_size, sizeof new_strings_size,
		      (unsigned char *) &new_strings_size);
#endif
  mywrite (outdesc, &new_strings_size, sizeof new_strings_size, outname);
#ifdef HOST_TARGET_ENDIANESS_DIFFERS
  new_strings_size = md_chars_to_number ((unsigned char *) &new_strings_size,
					 sizeof new_strings_size);
#endif

  /* Write the string table.  */
  mywrite (outdesc, new_strings, new_strings_size, outname);

  if (mapelt->info.size & 1)
    mywrite (outdesc, "", 1, outname);
}

void
read_old_symdefs (map, archive_indesc)
     struct mapelt *map;
     int archive_indesc;
{
  struct mapelt *mapelt;
  char *data;
  int val;
  int symdefs_size;

  mapelt = find_mapelt_noerror (map, "__.SYMDEF");
  if (!mapelt)
    abort ();			/* Only call here if an old one exists */

  data  = (char *) xmalloc (mapelt->info.size);
  lseek (archive_indesc, mapelt->info.data_offset, 0);
  val = read (archive_indesc, data, mapelt->info.size);

#ifdef HOST_TARGET_ENDIANESS_DIFFERS
  symdefs_size = md_chars_to_number ((unsigned char *) data,
				     sizeof symdefs_size);
#else
  symdefs_size = *(unsigned long int *) data;
#endif
  original_num_symdefs = symdefs_size / sizeof (struct symdef);
  old_symdefs = (struct symdef *) (data + sizeof (symdefs_size));
#ifdef HOST_TARGET_ENDIANESS_DIFFERS
  md_chars_to_symdefs ((unsigned char *) old_symdefs,
		       (struct md_symdef *) old_symdefs, original_num_symdefs);
#endif
  old_strings = ((char *) (old_symdefs + original_num_symdefs)
		 + sizeof (symdefs_size));
#ifdef HOST_TARGET_ENDIANESS_DIFFERS
  old_strings_size
    = md_chars_to_number ((unsigned char *) (old_symdefs
					     + original_num_symdefs),
			  sizeof old_strings_size);
#else
  old_strings_size
    = *(unsigned long int *) (old_symdefs + original_num_symdefs);
#endif
}

/* Read various information from the header of an object file.
   Return 0 for failure or 1 for success.  */

int
read_header_info (mapelt, desc, offset, syms_offset, syms_size, strs_offset, strs_size)
     struct mapelt *mapelt;
     int desc;
     long int offset;
     long int *syms_offset;
     unsigned int *syms_size;
     long int *strs_offset;
     unsigned int *strs_size;
{
    struct exec hdr;
    int len;

    lseek (desc, offset, 0);
#ifdef HEADER_SEEK_FD
    HEADER_SEEK_FD (desc);
#endif

    len = read (desc, (char *) &hdr, sizeof hdr);
#ifdef HOST_TARGET_ENDIANESS_DIFFERS
    md_chars_to_hdr ((unsigned char *) &hdr, &hdr);
#endif
    if (len == sizeof hdr && !N_BADMAG(hdr))
      {
	*syms_offset = N_SYMOFF (hdr);
	*syms_size = hdr.a_syms;
	*strs_offset = N_STROFF (hdr);
	lseek (desc, N_STROFF (hdr) + offset, 0);
	if (read (desc, (char *) strs_size, sizeof *strs_size) != sizeof *strs_size)
	  {
	    error_with_file ("failure reading string table size in ", mapelt);
	    return 0;
	  }
#ifdef HOST_TARGET_ENDIANESS_DIFFERS
	*strs_size = md_chars_to_number ((unsigned char *) strs_size,
					 sizeof *strs_size);
#endif
	return 1;
      }

  error_with_file ("bad format (not an object file) in ", mapelt);
  return 0;
}

/* Create the info.symdefs for a new member
   by reading the file it is coming from.  */

void
make_new_symdefs (mapelt, archive_indesc)
     struct mapelt *mapelt;
     int archive_indesc;
{
  int indesc;
  char *name = mapelt->info.name;
  long int syms_offset, strs_offset;
  unsigned int syms_size, strs_size;
  struct nlist *symbols;
  int symcount;
  char *strings;
  register unsigned int i;
  unsigned long int offset;

  if (name == 0)
    /* Deleted member.  */
    abort ();

  if (mapelt->info.offset != 0)
    {
      indesc = archive_indesc;
      lseek (indesc, mapelt->info.data_offset, 0);
      offset = mapelt->info.data_offset;
    }
  else
    {
      indesc = open (mapelt->info.name, 0, 0);
      if (indesc < 0)
	{
	  perror_with_name (mapelt->info.name);
	  return;
	}
      offset = 0;
    }

  if (!read_header_info (mapelt, indesc, offset, &syms_offset, &syms_size, &strs_offset, &strs_size))
    {
      if (mapelt->info.offset == 0)
	close (indesc);
      return;
    }

  /* Number of symbol entries in the file.  */
  symcount = syms_size / sizeof (struct nlist);
  /* Allocate temporary space for the symbol entries.  */
  symbols = (struct nlist *) alloca (syms_size);
  /* Read in the symbols.  */
  lseek (indesc, syms_offset + offset, 0);
  if (read (indesc, (char *) symbols, syms_size) != syms_size)
    {
      error_with_file ("premature end of file in symbols of ", mapelt);
      if (mapelt->info.offset == 0)
	(void) close (indesc);
      return;
    }
#ifdef HOST_TARGET_ENDIANESS_DIFFERS
  md_chars_to_nlist ((unsigned char *) symbols, symbols, symcount);
#endif

  /* The string table size includes the size word.  */
  if (strs_size < sizeof (strs_size))
    {
      error_with_file ("bad string table size in ", mapelt);
      if (mapelt->info.offset == 0)
	(void) close (indesc);
      return;
    }
  strs_size -= sizeof (strs_size);

  /* Allocate permanent space for the string table.  */
  strings = (char *) xmalloc (strs_size);

  /* Read in the strings.  */
  lseek (indesc, offset + strs_offset + sizeof strs_size, 0);
  if (read (indesc, strings, strs_size) != strs_size)
    {
      error_with_file ("premature end of file in strings of ", mapelt);
      if (mapelt->info.offset == 0)
	(void) close (indesc);
      return;
    }

  if (indesc != archive_indesc)
    (void) close (indesc);

  /* Discard the symbols we don't want to mention; compact the rest down.  */
  symcount = filter_symbols (symbols, symcount);

  mapelt->info.symdefs = (struct symdef *)
    xmalloc (symcount * sizeof (struct symdef));
  mapelt->info.nsymdefs = symcount;
  mapelt->info.string_size = 0;

  for (i = 0; i < symcount; ++i)
    {
      unsigned long int stroff = symbols[i].n_un.n_strx - sizeof (strs_size);
      char *symname = strings + stroff;
      if (stroff > strs_size)
	{
	  char buf[100];
	  sprintf (buf, "ridiculous string offset %lu in symbol %u of ",
		   stroff + sizeof (strs_size), i);
	  error_with_file (buf, mapelt);
	  return;
	}
      mapelt->info.symdefs[i].s.name = symname;
      mapelt->info.string_size += strlen (symname) + 1;
    }
}

/* Choose which symbol entries to mention in __.SYMDEF;
   compact them downward to get rid of the rest.
   Return the number of symbols left.  */

int
filter_symbols (syms, symcount)
     struct nlist *syms;
     unsigned int symcount;
{
  struct nlist *from, *to;
  struct nlist *end = syms + symcount;

  for (to = from = syms; from < end; ++from)
    if ((from->n_type & N_EXT)
	&& (from->n_type != N_EXT || from->n_value != 0))
      *to++ = *from;

  return to - syms;
}


/* Update the __.SYMDEF data before writing a new archive.  */

void
update_symdefs (map, archive_indesc)
     struct mapelt *map;
     int archive_indesc;
{
  struct mapelt *tail;
  int pos;
  register unsigned int i;
  unsigned int len;
  struct symdef *s;
  unsigned long int deleted_strings_size = 0;

  nsymdefs = original_num_symdefs;
  num_old_symdefs = original_num_symdefs;
  new_strings_size = old_strings_size;

  if (nsymdefs != 0)
    {
      /* We already had a __.SYMDEF member, so just update it.  */

      /* Mark as canceled any old symdefs for members being deleted.  */

      for (tail = map; tail != 0; tail = tail->next)
	{
	  if (tail->info.name == 0)
	    {
	      /* Old member being deleted.  Delete its symdef entries too.  */
	      for (i = 0; i < original_num_symdefs; i++)
		if (old_symdefs[i].offset == tail->info.offset)
		  {
		    old_symdefs[i].offset = 0;
		    --nsymdefs;
		    deleted_strings_size
		      += strlen (old_strings
				 + old_symdefs[i].s.stringoffset) + 1;
		  }
	    }
	}

      /* Compactify old symdefs.  */
      {
	register unsigned int j = 0;
	for (i = 0; i < num_old_symdefs; ++i)
	  {
	    if (j != i)
	      old_symdefs[j] = old_symdefs[i];
	    if (old_symdefs[i].offset != 0)
	      ++j;
	  }
	num_old_symdefs -= i - j;
      }

      /* Create symdef data for any new members.  */
      for (tail = map; tail != 0; tail = tail->next)
	{
	  if (tail->info.offset != 0
	      || tail->info.name == 0
	      || !strcmp (tail->info.name, "__.SYMDEF"))
	    continue;
	  make_new_symdefs (tail, archive_indesc);
	  nsymdefs += tail->info.nsymdefs;
	  new_strings_size += tail->info.string_size;
	}
    }
  else
    {
      /* Create symdef data for all existing members.  */

      for (tail = map; tail != 0; tail = tail->next)
	{
	  if (tail->info.name == 0
	      || !strcmp (tail->info.name, "__.SYMDEF"))
	    continue;
	  make_new_symdefs (tail, archive_indesc);
	  nsymdefs += tail->info.nsymdefs;
	  new_strings_size += tail->info.string_size;
	}
    }

  new_strings_size -= deleted_strings_size;
  old_strings_size -= deleted_strings_size;

  /* Now we know the size of __.SYMDEF,
     so assign the positions of all the members.  */

  tail = find_mapelt_noerror (map, "__.SYMDEF");
  tail->info.size = (sizeof (nsymdefs) + (nsymdefs * sizeof (struct symdef))
		     + sizeof (new_strings_size) + new_strings_size);
  symdef_mapelt = tail;

  pos = SARMAG;
  for (tail = map; tail != 0; tail = tail->next)
    {
      if (tail->info.name == 0)
	/* Ignore deleted members.  */
	continue;
      tail->info.new_offset = pos;
      pos += sizeof (struct ar_hdr) + tail->info.size;
      if (tail->info.size & 1)
	++pos;
    }

  /* Now update the offsets in the symdef data
     to be the new offsets rather than the old ones.  */

  for (tail = map; tail != 0; tail = tail->next)
    {
      if (tail->info.name == 0)
	continue;
      if (tail->info.symdefs == 0)
	/* Member without new symdef data.
	   Check the old symdef data; it may be included there. */
	for (i = 0; i < num_old_symdefs; i++)
	  {
	    if (old_symdefs[i].offset == tail->info.offset)
	      old_symdefs[i].offset = tail->info.new_offset;
	  }
      else
	for (i = 0; i < tail->info.nsymdefs; i++)
	  tail->info.symdefs[i].offset = tail->info.new_offset;
    }

  /* Generate new, combined string table and put each string's offset into the
     symdef that refers to it.  Note that old symdefs ref their strings by
     offsets into old_strings but new symdefs contain addresses of strings.  */

  new_strings = (char *) xmalloc (new_strings_size);
  pos = 0;

  /* Write the strings of the old symdefs and update the structures
     to contain indices into the string table instead of strings.  */
  for (i = 0; i < num_old_symdefs; i++)
    {
      strcpy (new_strings + pos, old_strings + old_symdefs[i].s.stringoffset);
      old_symdefs[i].s.stringoffset = pos;
      pos += strlen (new_strings + pos) + 1;
    }
  if (pos < old_strings_size)
    {
      unsigned int d = old_strings_size - pos;
      /* Correct the string table size.  */
      new_strings_size -= d;
      /* Correct the size of the `__.SYMDEF' member,
	 since it contains the string table.  */
      symdef_mapelt->info.size -= d;
    }
  else if (pos > old_strings_size)
    fatal ("Old archive's string size was %u too small.",
	   (void*)(pos - old_strings_size));

  for (tail = map; tail != 0; tail = tail->next)
    if (tail->info.symdefs)
      {
	len = tail->info.nsymdefs;
	s = tail->info.symdefs;

	for (i = 0; i < len; i++)
	  {
	    strcpy (new_strings + pos, s[i].s.name);
	    s[i].s.stringoffset = pos;
	    pos += strlen (new_strings + pos) + 1;
	  }
      }
  if (pos != new_strings_size)
    fatal ("internal error: inconsistency in new_strings_size", 0);
}


/* Print error message and usage message, and exit.  */

void
usage (s1, val)
     char *s1;
     int val;
{
  char vbuf[16];
  sprintf(vbuf, "%d", val);
  error (s1, vbuf);
  fprintf (stderr, "\
Usage: %s [d|m|p|q|r|t|x [[abi [position-name] [cilouv]] archive file...\n",
	   program_name);
  exit (1);
}

/* Print error message and exit.  */

void
fatal (s1, s2)
     char *s1, *s2;
{
  error (s1, s2);
  exit (1);
}

/* Print error message.  `s1' is printf control string, the rest are args.  */

void
error (s1, s2)
     char *s1, *s2;
{
  fprintf (stderr, "%s: ", program_name);
  fprintf (stderr, s1, s2);
  fprintf (stderr, "\n");
}

void
error3 (s1, s2, s3)
     char *s1, *s2, *s3;
{
  fprintf (stderr, "%s: ", program_name);
  fprintf (stderr, s1, s2, s3);
  fprintf (stderr, "\n");
}

void
error_with_file (string, mapelt)
     char *string;
     struct mapelt *mapelt;
{
  fprintf (stderr, "%s: ", program_name);
  fprintf (stderr, string);
  if (mapelt->info.offset != 0)
    fprintf (stderr, "%s(%s)", archive, mapelt->info.name);
  else
    fprintf (stderr, "%s", mapelt->info.name);
  fprintf (stderr, "\n");
}

void
perror_with_name (name)
     char *name;
{
  error (concat ("", strerror(errno), " for %s"), name);
}

void
pfatal_with_name (name)
     char *name;
{
  fatal (concat ("", strerror(errno), " for %s"), name);
}

/* Return a newly-allocated string whose contents
   concatenate those of S1, S2, and S3.  */

char *
concat (s1, s2, s3)
     const char *s1, *s2, *s3;
{
  int len1 = strlen (s1), len2 = strlen (s2), len3 = strlen (s3);
  char *result = (char *) xmalloc (len1 + len2 + len3 + 1);

  strcpy (result, s1);
  strcpy (result + len1, s2);
  strcpy (result + len1 + len2, s3);
  *(result + len1 + len2 + len3) = 0;

  return result;
}

/* Like malloc but get fatal error if memory is exhausted.  */

char *
xmalloc (size)
     unsigned int size;
{
#ifdef MALLOC_0_RETURNS_NULL
  char *result = malloc (size ? size : 1);
#else
  char *result = malloc (size);
#endif
  if (result == 0)
    fatal ("virtual memory exhausted", 0);
  return result;
}

char *
xrealloc (ptr, size)
     char *ptr;
     unsigned int size;
{
  char *result = realloc (ptr, size);
  if (result == 0)
    fatal ("virtual memory exhausted", 0);
  return result;
}


#ifndef HAVE_RENAME
int
rename (from, to)
     char *from, *to;
{
  (void)unlink (to);
  if (link (from, to) < 0
      || unlink (from) < 0)
    return -1;
  else
    return 0;
}
#else
#ifdef ATT
int Error;
#endif
#endif
