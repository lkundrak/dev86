/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This program is distributed under the GNU General Public License.
 */

/*
 * File Tool, This program is a collection of basic file tools
 * it includes cat, cp, ln, mkdir, mknod, chmod, chown, mv, rm
 *
 * Links may be used to call it under any of these names.
 */
#include <stdio.h>
#ifdef __STDC__
#include <unistd.h>
#include <stdlib.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/param.h>
#include <utime.h>
#include <pwd.h>
#include <grp.h>

#if defined(S_IFSOCK) && !defined(__BCC__)
#define __HAS_SOCKETS
#endif
#ifndef S_IFLNK
#define lstat stat
#endif

#ifdef __HAS_SOCKETS
#include <sys/socket.h>
#endif

/* Ansi prototypes */
#ifdef __STDC__
#define PR(x) x
#else
#define PR(x) ()
#endif

int main PR((int argc, char ** argv));
int select_command PR((char * argv));
void do_prep PR((void));
void do_post PR((void));
void execute PR((char * dname, char * fname));
int exec_for_subdir PR((char * dname));
void exec_for_item PR((int when, char * fname));
void parse_perms PR((char * prefix, char * ustring));
int edit_mode PR((int mode, char * mode_str));
int cmd_ft PR((char * fname));
int cmd_mkfifo PR((char * fname));
int cmd_mksock PR((char * fname));
int cmd_rm PR((char * fname));
void build_dest PR((char * dest, char * name, char * newpath));
int strisdigit PR((char * str));
int cmd_mv PR((char * fname));
int cmd_ln PR((char * fname));
int cmd_cp PR((char * fname));
int copy_modes PR((char * file));
int copy_file PR((char * source, char * dest));
void Usage PR((void));
int cmd_mkdir PR((char * dirname));
int cmd_mknod PR((void));
int warning PR((int enumber, char * estr, char * eobj));
int error PR((int enumber, char * estr, char * eobj));

#define DO_BDIR         0x0010	/* Do Dir before contents */
#define DO_ADIR         0x0020	/* Do Dir after contents */
#define DO_MCOPY        0x0040	/* Preserve modes flag forced */
#define OK_DIR          0x0080	/* Directorys OK even if no flg_recurse */
#define IGN_LNK         0x0100	/* Not interested in symlinks */
#define NO_SOURCE       0x0200	/* Named files created */
#define OK_NO_SOURCE    0x0400	/* Don't need the source */

#define CMD_FT		(0+OK_DIR+DO_BDIR)
#define CMD_CAT		(1+IGN_LNK)
#define CMD_CHGRP	(2+OK_DIR+IGN_LNK+DO_ADIR)
#define CMD_CHMOD	(3+OK_DIR+IGN_LNK+DO_ADIR)
#define CMD_CHOWN	(4+OK_DIR+IGN_LNK+DO_ADIR)
#define CMD_CP		(5+IGN_LNK)
#define CMD_EXTAR       (6+DO_MCOPY+DO_BDIR)
#define CMD_INSTALL     (7+DO_MCOPY)
#define CMD_LN		(8+IGN_LNK+DO_BDIR)
#define CMD_MKDIR	(9+NO_SOURCE)
#define CMD_MKFIFO	(10+NO_SOURCE)
#define CMD_MKSOCK	(11+NO_SOURCE)
#define CMD_MKNOD	(12+NO_SOURCE)
#define CMD_MV		(13+DO_MCOPY+OK_DIR+DO_BDIR)
#define CMD_RM		(14+DO_ADIR)

struct {
   char * name;
   int cmd;
   int argpat;
   char * opts;
} command_list[] = 
{
   { "ft",	CMD_FT,       0, "-Rv" },
   { "cat",	CMD_CAT,      0, "uR" },
   { "chgrp",	CMD_CHGRP,    1, "vfR" },
   { "chmod",	CMD_CHMOD,    1, "vfR" },
   { "chown",	CMD_CHOWN,    1, "vfR" },
   { "cp",	CMD_CP,      -1, "vifRrpsda" },
   { "extar",	CMD_EXTAR,    1, "" },
   { "install", CMD_INSTALL, -1, "cdso:g:m:" },
   { "ln",	CMD_LN,      -1, "vifs" },
   { "mkdir",	CMD_MKDIR,    0, "m:" },
   { "mkfifo",	CMD_MKFIFO,   0, "m:" },
#ifdef __HAS_SOCKETS
   { "mksock",	CMD_MKSOCK,   0, "m:" },
#endif
   { "mknod",	CMD_MKNOD,    4, "m:" },
   { "mv",	CMD_MV,      -1, "vif"  },
   { "rm",	CMD_RM,       0, "vifr" },
   { 0 }
};

int cmd_arg = 0;
int cmd_tok = CMD_FT;
char * cmd_opt = "-";
char * cmd_string = 0;	/* the first (or last) arg where special */
char * prog_name = "";

char ** flist = 0;
int     fcount = 0;
int     add_base=0;
char *  or_name = 0;
int     or_offset = 0;

int flg_recurse = 0;
int flg_verbose = 1;
int flg_preserve= 0;
int flg_mkpdir  = 0;
int flg_noderef = 0;
int flg_symlink = 0;
int flg_exestrip= 0;

int flg_r, flg_force;
char *str_o, *str_g, *str_m;

/* Things to set on the new file */
int set_user    = -1;
int set_group   = -1;
int set_mode    = -1;
time_t set_time = -1;
char mode_str[32] = "";
int u_mask      = 0;    /* 07777 altered by umask() */

struct stat cur_file_stat;
struct stat dest_item;
struct stat access_stat;

int done_something = 0;

int
main(argc, argv)
int argc; char ** argv;
{
   int ar;
   (void) select_command(argv[0]);

   for(ar=1;
       argv[ar] && argv[ar][0] == '-' && argv[ar][1];
       ar++)
   {
      char * p = argv[ar]+1;
      /* For symbolic changes of the form -rwx */
      if( cmd_tok == CMD_CHMOD && strchr("rwx", *p) != 0 ) break;
      while(*p)
      {
         char * ap=0, *av=0;
	 char ch;
         /* Is it a valid opt for this cmd */
         if(*p == ':' || (ap=strchr(cmd_opt, *p)) == 0) Usage();

	 /* Got an argument ? */
	 if(ap[1] == ':')
	 {
	    if(!argv[ar+1]) Usage();
	    av = argv[++ar];
         }

	 if( (ch = *p) == '-' )
	 {
            if( (ch=select_command(p)) < 0 ) Usage();
	 }
         switch(ch)
         {
	 case '\0': break;
	 case 'r':
	 case 'R': flg_recurse++; break;
	 case 'v': flg_verbose++; break;
	 case 'p': if(cmd_tok == CMD_MKDIR) flg_mkpdir++;
	           else                     flg_preserve++;
		   break;
	 case 'd': if(cmd_tok == CMD_INSTALL)
	           { flg_mkpdir++; cmd_arg=0; } /* Special mkdir */
		   else flg_noderef++;          /* cmd_copy */
	           break;

	 case 'f': flg_force++; flg_verbose=0; break;
	 case 'o': str_o = av; break;
	 case 'g': str_g = av; break;
	 case 'm': str_m = av; break;

	 case 's': flg_symlink++;
	           if( cmd_tok == CMD_LN) cmd_tok |= OK_DIR+OK_NO_SOURCE;
		   break;
	 case 'a': flg_recurse++; flg_preserve++; flg_noderef++;
	           break;
         }
	 if(*p == '-') break;
	 p++;
      }
   }

   switch(cmd_arg)
   {
   case 1:
      if( ar >= argc ) Usage();
      cmd_string = argv[ar++];
      fcount = argc-ar;
      flist = argv+ar;
      break;
   case 0:
      fcount = argc-ar;
      flist = argv+ar;
      break;
   case -1:
      if( ar >= argc ) Usage();
      cmd_string = argv[argc-1];
      fcount = argc-ar-1;
      flist = argv+ar;
      break;
   default:
      if( ar != argc-cmd_arg ) Usage();
      fcount = argc-ar;
      flist = argv+ar;
      break;
   }

   do_prep();

   for(ar=0; ar<fcount; ar++)
   {
      done_something=1;
      or_name = flist[ar]; or_offset = strlen(or_name)+1;
      execute(flist[ar], (char*)0);
   }

   do_post();

   if( !done_something )
   {
      if( cmd_tok == CMD_CAT )
         execute("-", (char*)0);
      else
         Usage();
   }
   return 0;
}

int select_command(argv)
char * argv;
{
   int ar;
   char *p, *s;
   prog_name = argv;
   for(ar=0; command_list[ar].name; ar++)
   {
      p = strrchr(argv, '-'); if(p) p++; else p=argv;
      s = strrchr(p, '/'); if(s) s++; else s=p;
      if( strcmp(s, command_list[ar].name) == 0 )
      {
         cmd_arg = command_list[ar].argpat;
	 cmd_tok = command_list[ar].cmd;
	 cmd_opt = command_list[ar].opts;
         return 0;
      }
   }
   return -1;
}

void do_prep()
{
   char * prefix = "::";

   u_mask = umask(077);
   umask(u_mask);
   u_mask = (07777&(~u_mask));

   if(cmd_tok&DO_MCOPY)    flg_preserve++;
   if(str_m) parse_perms(prefix,   str_m);

   switch(cmd_tok)
   {
   /* mknod is very different */
   case CMD_MKNOD: cmd_mknod(); exit(0); break;

   case CMD_CP:
                if(strcmp(cmd_string, "-") == 0)
                {
                   cmd_tok = CMD_CAT;
                   cmd_arg = 0;
		   break;
                }
		if(flg_symlink)
		{
		   cmd_tok = CMD_LN+OK_DIR+OK_NO_SOURCE;
		   flg_preserve = 0;
		}
                break;

   case CMD_CHOWN: prefix++;
   case CMD_CHGRP: prefix++;
   case CMD_CHMOD:
                   parse_perms(prefix, cmd_string);
		   set_time = 0;
                   break;
   case CMD_INSTALL:
		   flg_exestrip = flg_symlink;
		   flg_symlink = 0;
                   if(str_o) parse_perms(prefix+2, str_o);
                   if(str_g) parse_perms(prefix+1, str_g);
		   if(flg_mkpdir) cmd_tok = CMD_MKDIR;
		   else
		   {
		      cmd_tok = CMD_CP;
		      flg_preserve = 1;
		   }
		   break;
   }

#ifndef S_IFLNK
   if(flg_symlink)
   {
      error(0, "No support for symlinks available:", cmd_string);
      exit(1);
   }
#endif

   /* Are we transfering many to one ? Then it must be a directory */
   if(cmd_arg == -1)
   {
      if( stat(cmd_string, &dest_item) == -1)
      {
         if( fcount > 1 )
	 {
            if( cmd_mkdir(cmd_string) < 0 )
	       exit(1);
            stat(cmd_string, &dest_item);
	    add_base = 1;
	 }
      }
      else
      {
         if( !S_ISDIR(dest_item.st_mode) )
	 {
	    if( fcount > 1 )
	    {
	       error(0, "Destination must be a directory:", cmd_string);
	       exit(1);
	    }
	 }
	 else add_base = 1;
      }
   }
}

void do_post()
{
   /* Oh! It seems there's nothing to do, ah well. */
}

void execute(dname, fname)
char * dname; char * fname;
{
   char * buf;
   if( strcmp(dname, "-") == 0 ) 
   {
      exec_for_item(0, dname);
      return;
   }
   if( fname )
   {
      buf = alloca(strlen(dname) + strlen(fname) + 4);
      if( buf == 0 )
      {
         error(errno, "Can't allocate memory for path beyond ", dname);
	 return ;
      }
      strcpy(buf, dname);
      if(strcmp(dname, "/")) strcat(buf, "/");
      strcat(buf, fname);
   }
   else buf = dname;

   if( lstat(buf, &cur_file_stat) == -1 )
   {
      if( cmd_tok&(NO_SOURCE|OK_NO_SOURCE) )
	 exec_for_item(0, buf);
      else 
	 warning(errno, "", buf);
      return;
   }
   if( !flg_force && ( cmd_tok&NO_SOURCE ))
   {
      error(EEXIST, "", buf);
      return;
   }

   if( S_ISDIR(cur_file_stat.st_mode))
   {
      if( (cmd_tok&OK_DIR) || flg_recurse )
	 (void) exec_for_subdir(buf);
      else
	 error(EISDIR, "", buf);
      return;
   }

#ifdef S_IFLNK
   if( S_ISLNK(cur_file_stat.st_mode))
   {
      /* Links are special */
      if( cmd_tok&IGN_LNK )
      {
	 if( stat(buf, &cur_file_stat) == -1 )
	 {
	    warning(errno, "", buf);
	    return;
	 }
      }
   }
#endif
   exec_for_item(0, buf);
}

int exec_for_subdir(dname)
char * dname;
{
   DIR * dfd;
   struct dirent * ent;
   int  old_mode = -1;

   if( cmd_tok&DO_BDIR ) exec_for_item(-1, dname);

   if( flg_recurse )
   {
      dfd = opendir(dname);
      if( dfd == 0 && errno == EACCES && flg_force )
      {
	 old_mode = (cur_file_stat.st_mode & 07777);
	 if( chmod(dname, (0700|old_mode)) )
	    return error(errno, "", dname);

	 dfd = opendir(dname);
      }
      if( dfd == 0 ) return error(errno, "", dname);

      while((ent=readdir(dfd)))
      {
	 if( strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0 )
	    continue;

	 alloca(0);	/* Free up if using fake version */
	 execute(dname, ent->d_name);
      }
      closedir(dfd);
      if( old_mode != -1 )
	 chmod(dname, old_mode);
   }

   if( cmd_tok&DO_ADIR )
   {
      lstat(dname, &cur_file_stat);
      exec_for_item(1, dname);
   }
   return 0;
}

void exec_for_item(when, fname)
int when; char * fname;
{
   int rv = -1;
   switch(cmd_tok)
   {
   case CMD_FT:      rv = cmd_ft(fname); break;

   case CMD_CAT:     rv = copy_file(fname, "-"); break;

   case CMD_CHGRP:   /* And fall */
   case CMD_CHMOD:   /* And fall */
   case CMD_CHOWN:   rv = copy_modes(fname); break;

   case CMD_CP:      rv = cmd_cp(fname); break;
   case CMD_MV:      rv = cmd_mv(fname); break;
   case CMD_RM:      rv = cmd_rm(fname); break;

   case CMD_EXTAR:   error(EINVAL, "", "No code."); exit(1);

   case CMD_LN+OK_DIR+OK_NO_SOURCE:
   case CMD_LN:      rv = cmd_ln(fname); break;

   case CMD_INSTALL: error(EINVAL, "", ""); exit(1);

   case CMD_MKDIR:   rv = cmd_mkdir(fname);  break;
   case CMD_MKFIFO:  rv = cmd_mkfifo(fname); break;
#ifdef __HAS_SOCKETS
   case CMD_MKSOCK:  rv = cmd_mksock(fname); break;
#endif
   case CMD_MKNOD:   break;
   }
}

void parse_perms(prefix, ustring)
char * prefix; char * ustring;
{
   char * userstr;
   char * groupstr;
   char * modestr;
   char * cp;
   struct passwd * pwd = 0;
   struct group  * grp;

   userstr = alloca(strlen(prefix) + strlen(ustring) + 2);
   strcpy(userstr, prefix);
   strcat(userstr, ustring);

   /* Select User */
   cp = strchr(userstr, ':');
   if(!cp) cp = strchr(userstr, '.');
   if(cp) *cp = '\0';

   /* If there's a user */
   if( *userstr != 0 )
   {
      pwd = getpwnam(userstr);
      if(pwd == NULL)
      {
         if(!strisdigit(userstr) )
	 {
	    error(EINVAL, "Unknown user ", userstr);
	    exit(1);
	 }
	 set_user = atoi(userstr);
      }
      else set_user = pwd->pw_uid;
      endpwent();
   }
   if(cp)
   {
      groupstr = cp+1;
      cp = strchr(groupstr, ':');
      if(!cp) cp = strchr(groupstr, '.');
      if(cp) *cp = '\0';
      if( *groupstr != '\0' )
      {
         grp = getgrnam(groupstr);
         if(grp == NULL)
         {
            if(!strisdigit(groupstr) )
	    {
	       error(EINVAL, "Unknown group ", groupstr);
	       exit(1);
	    }
	    set_group = atoi(groupstr);
         }
         else set_group = grp->gr_gid;
	 endgrent();
      }
      else if( pwd )
         set_group = pwd->pw_gid;
   }
   if(cp)
   {
      modestr = cp+1;
      if(strisdigit(modestr))
         set_mode = strtol(modestr, NULL, 8);
      else
      {
         strncpy(mode_str, modestr, sizeof(mode_str)-1);
	 /* This is the time that the mode change will fail on syn error */
	 (void) edit_mode(u_mask, mode_str);
      }
   }

   if( set_user == -1 && set_group == -1 && set_mode < 0 && *mode_str == 0)
   {
      error(EINVAL, "", "Permission string has no changes");
      exit(1);
   }
}

int edit_mode(mode, mode_str)
int mode; char * mode_str;
{
   char * str=mode_str;
static mtab[] = {0, 0111, 0222, 0333, 0444, 0555, 0666, 0777 };

   int done_change = 0;
   int isdir = S_ISDIR(mode);
   int change_op   = 0;
   int change_mask = u_mask;
   int v=0, s=0, nm=0;

   for(; *mode_str; mode_str++)
   {
      switch(*mode_str)
      {
      case ',': change_op = 0;
                change_mask=u_mask; continue;
      case '=': change_op = 1;      if(0) {
      case '+': change_op = 2;    } if(0) {
      case '-': change_op = 3;    }
		v=0; nm=0;
                if(strchr(",=+-", mode_str[1]) == 0 ) continue;
		break;
      case 'a': if(change_op) goto ch_error;
                nm |= 07777;         if(0) {
      case 'u': nm |= 04700; s= 6; } if(0) {
      case 'g': nm |= 02070; s= 3; } if(0) {
      case 'o': nm |= 01007; s= 0; }
                if(change_op==0) { change_mask=nm; continue; }
		v |= mtab[(mode>>s)&7];
                break;
      case 'r': v |= 0444; break;
      case 'w': v |= 0222; break;
      case 'x': v |= 0111; break;
      case 's': v |=06000; break;
      case 't': v |=01000; break;
      case 'X': v |= mtab[isdir]; break;
      default: goto ch_error;
      }
      switch(change_op)
      {
      case 0: goto ch_error;
      case 1: mode= ((mode&(~change_mask)) | (v&change_mask));
	      break;
      case 2: mode= ( mode |  (v&change_mask));
	      break;
      case 3: mode= ( mode & ~(v&change_mask));
	      break;
      }
      done_change=1;
   }
   if(!done_change)
   {
ch_error:
      error(EINVAL, "Invalid mode string ", str);
      exit(1);
   }
   return mode;
}

int
cmd_ft(fname)
char * fname;
{
static char oldpath[2048] = "~";
static int last_uid=-1, last_gid=-1, last_mode=-1;
   struct passwd * pptr;
   struct group  * gptr;
   int major, minor;

   if( flg_verbose>1 )
   {
      char *p = 0;
      if( fname[1] ) p = strrchr(fname, '/');
      if( p )
      {
         *p = '\0';
	 if( strcmp(fname, oldpath) != 0 )
	 {
	    strcpy(oldpath, fname);
	    printf("%s/\n", oldpath);
	 }
         *p = '/';
      }
      else if( *oldpath )
	 *oldpath = '\0';
      if(p) printf("%s", p+1);
      else  printf("%s", fname);

#ifdef S_IFLNK
      if( S_ISLNK(cur_file_stat.st_mode))
      {
         char linkbuf[1024];
	 int v;
	 *linkbuf='\0';
	 v = readlink(fname, linkbuf, sizeof(linkbuf));
	 if(v>0) linkbuf[v] = '\0';
         printf("\t+%s", linkbuf);
      }
      else
#endif
      if( cur_file_stat.st_mode != last_mode
       || cur_file_stat.st_uid != last_uid
       || cur_file_stat.st_gid != last_gid)
      {
         printf("\t");
         if( cur_file_stat.st_uid != last_uid )
	 {
	    pptr = getpwuid(cur_file_stat.st_uid);
	    if( pptr )
	       printf("%s", pptr->pw_name);
	    else
	       printf("%d", cur_file_stat.st_uid);
	 }
         printf(":");
         if( cur_file_stat.st_gid != last_gid )
	 {
	    gptr = getgrgid(cur_file_stat.st_gid);
	    if( gptr )
	       printf("%s", gptr->gr_name);
	    else
	       printf("%d", cur_file_stat.st_gid);
	 }
         if( (cur_file_stat.st_mode&07777) != (last_mode&07777) )
	    printf(":%03o", cur_file_stat.st_mode & 07777);

#ifdef __linux__
	 major = ((cur_file_stat.st_rdev >> 8) & 0xfff);
	 if (sizeof(cur_file_stat.st_rdev) > 4)
	    major |= ((cur_file_stat.st_rdev >> 32) & ~0xfff);
	 minor = (cur_file_stat.st_rdev & 0xff) | 
	         ((cur_file_stat.st_rdev >> 12) & ~0xff);
#else
	 major = ((cur_file_stat.st_rdev >> 8) & 0xFF);
	 minor = (cur_file_stat.st_rdev&0xFF);
#endif
         switch(cur_file_stat.st_mode & S_IFMT)
         {
         case S_IFDIR:  printf("\td"); break;
         case S_IFIFO:  printf("\tp"); break;
#ifdef __HAS_SOCKETS
         case S_IFSOCK: printf("\ts"); break;
#endif
         case S_IFBLK:  printf("\tb,%d,%d", major, minor);
		        break;
         case S_IFCHR:  printf("\tc,%d,%d", major, minor);
		        break;
         }
         last_mode = ((cur_file_stat.st_mode&07777)|S_IFREG);
         if( (cur_file_stat.st_mode&07000) ) last_mode = -1;
         last_uid = cur_file_stat.st_uid;
         last_gid = cur_file_stat.st_gid;
      }
      printf("\n");
   }
   else printf("%s\n", fname);

   return 0;
}

int
cmd_mkfifo(fname)
char * fname;
{
   int rv;
   int mode=0666;
   if( set_mode >= 0 ) mode=set_mode;
   rv = mknod(fname, S_IFIFO|mode, 0);
   if(rv<0)
      warning(errno, "Cannot create fifo ", fname);
   return rv;
}

#ifdef __HAS_SOCKETS
int
cmd_mksock(fname)
char * fname;
{
   int rv, fd, len;
   struct sockaddr *adr;

   len = strlen(fname)+1 + sizeof(*adr) - sizeof(adr->sa_data);
   if( len < sizeof(*adr) ) len = sizeof(*adr);

   adr = alloca(len+2);
   adr->sa_family = AF_UNIX;
   strcpy(adr->sa_data, fname);

   rv = fd = socket(AF_UNIX, SOCK_STREAM, 0);
   if( fd>=0 ) rv = bind(fd, adr, len);
   if( fd>=0 ) close(fd);
   if(set_mode >= 0 && chmod(fname, set_mode&07777) < 0 )
      warning(errno, "Chmod ", fname);

   if(rv<0)
      warning(errno, "Cannot create socket ", fname);
   return rv;
}
#endif

int
cmd_rm(fname)
char * fname;
{
   struct stat dirstat;
   int rv;
   char * buf, * p;

   if( S_ISDIR(cur_file_stat.st_mode) )
      if( !flg_recurse ) return error(EISDIR, "", fname);

   if( S_ISDIR(cur_file_stat.st_mode) )
   {
      if( rmdir(fname) >= 0 ) return 0;
   }
   else
   {
      if( unlink(fname) >= 0 ) return 0;
   }

   if( !flg_force )
      return error(errno, "", fname);

   /* Try VERY hard */
   buf = alloca(strlen(fname)+4);
   strcpy(buf, fname);
   p = strrchr(buf, '/');
   if( p ) strcpy(p+1, "."); else strcpy(buf, ".");

   if( stat(buf, &dirstat) < 0 ) return -1;
   if( chmod(buf, dirstat.st_mode|0700) < 0 ) return -1;

   if( S_ISDIR(cur_file_stat.st_mode) )
      rv = rmdir(fname);
   else
      rv = unlink(fname);

   chmod(buf, dirstat.st_mode);

   return rv;
}

void
build_dest(dest, name, newpath)
char * dest; char * name; char * newpath;
{
   char * p;
   strcpy(dest, newpath);
   if( add_base )
   {
      strcat(dest, "/");
      p = strrchr(or_name, '/');
      if(p==0) strcat(dest, or_name);
      else     strcat(dest, p+1);
   }
   if(strlen(name) <= or_offset) return;
   strcat(dest, name+or_offset);
}

int
strisdigit(str)
char * str;
{
   if( str==0 || *str == 0 ) return 0;

   for(;*str; str++)
      if(*str>'9'|| *str<'0') return 0;
   return 1;
}

int
cmd_mv(fname)
char * fname;
{
   char * destfile;
   destfile = alloca(strlen(fname)+strlen(cmd_string)+4);

   build_dest(destfile, fname, cmd_string);

   if( !flg_force && lstat(destfile, &access_stat) == 0 )
      return error(EEXIST, "", destfile);

   if( rename(fname, destfile) == 0 ) return 0;

   if( errno != EXDEV )
      return error(errno, "", fname);

   if( S_ISDIR(cur_file_stat.st_mode) )
      return error(EISDIR, "Can't rename across devices ", fname);

   if( copy_file(fname, destfile) != 0 ) return -1;
   copy_modes(destfile);
   return unlink(fname);
}

int
cmd_ln(fname)
char * fname;
{
   char * destfile;
   destfile = alloca(strlen(fname)+strlen(cmd_string)+4);

   build_dest(destfile, fname, cmd_string);

   if( lstat(destfile, &access_stat) != -1 )
   {
      if( !flg_force ) return error(EEXIST, "", destfile);
      cmd_rm(destfile);
   }

#ifdef S_IFLNK
   if( flg_symlink )
   {
      if( symlink(fname, destfile) == 0 ) return 0;
   }
   else
   {
#endif
      if( link(fname, destfile) == 0 ) return 0;
#ifdef S_IFLNK
   }
#endif

   return error(errno, "", destfile);
}

int
cmd_cp(fname)
char * fname;
{
   struct stat dest_stat;
   char * destfile;
   int no_dest = 0;

   destfile = alloca(strlen(fname)+strlen(cmd_string)+4);

   build_dest(destfile, fname, cmd_string);

   if( stat(destfile, &dest_stat) >= 0 )
   {
      if( dest_stat.st_ino == cur_file_stat.st_ino
       && dest_stat.st_dev == cur_file_stat.st_dev )
      {
	 warning(EPERM, "Can't copy file to itself ", fname);
         return -1;
      }
   }
   else no_dest = 1;

   if( S_ISDIR(cur_file_stat.st_mode) )
   {
      if( !no_dest )
      {
         if( S_ISDIR(dest_stat.st_mode) ) return 0;
	 if( unlink(destfile) < 0 )
            return error(errno, "Can't delete ", destfile);
      }
      return cmd_mkdir(destfile);
   }
   else if( S_ISDIR(dest_stat.st_mode) )
      return error(EPERM, "Can't copy non-directory to directory ", destfile);
   else if( S_ISREG(cur_file_stat.st_mode) )
   { 
      /* Copy_ok - do we want to force a real file */;
      if( flg_force && !no_dest && !S_ISREG(dest_stat.st_mode) )
         cmd_rm(destfile);
   }
   else if( flg_recurse ) /* Don't copy other things while recursing */
   {
      return error(EPERM, "Can't copy ", fname);
   }

   if( copy_file(fname, destfile) != 0 ) return -1;
   if( flg_preserve ) copy_modes(destfile);
   return 0;
}

int
copy_modes(file)
char * file;
{
  int user, group, mode;
  /* chown turns off set[ug]id bits for non-root,
     so do the chmod last.  */

  /* Try to copy the old file's modtime and access time.  */
  if(set_time)
  {
    struct utimbuf tv;

    tv.actime = cur_file_stat.st_atime;
    tv.modtime = cur_file_stat.st_mtime;
    if( set_time != -1 )
      tv.modtime = set_time;
    if (utime (file, &tv) && !flg_force)
	return error (errno, "", file);
  }

  /* Try to preserve ownership.  For non-root it might fail, but that's ok.
     But root probably wants to know, e.g. if NFS disallows it.  */
  user  = cur_file_stat.st_uid; if(set_user != -1) user = set_user;
  group = cur_file_stat.st_gid; if(set_group != -1) group = set_group;

  if (chown (file, user, group)
      && (errno != EPERM || geteuid() == 0 || (flg_preserve==0 && flg_force==0)))
      error (errno, "Can't change perms for ", file);

  mode = cur_file_stat.st_mode;
  if(set_mode>=0) mode=set_mode;
  else if(*mode_str)
     mode = edit_mode(mode, mode_str);

  if (chmod (file, mode & 07777))
      return error (errno, "", file);

   return 0;
}

/* This copies from something to a file or stdout */
/* If the source has zero blocks (possibly holes) the destination
 * is built with holes (assuming it's a normal file) */

int
copy_file(source, dest)
char * source; char * dest;
{
   char * buf;
   int sfd, dfd;
   struct stat st;
   int blksz = BUFSIZ;
   int cc;
   char * ptr;
   int hole_flag = 0;
   int retv = 0;
   int no_seek;
   int mmode = 0666;

   if(flg_verbose>1) printf("%s -> %s\n", source, dest);
   if( strcmp(source, "-") == 0 )
      sfd = 0;
   else
   {
      sfd = open(source, O_RDONLY);
      if(sfd<0) return error(errno, "", source);
      mmode = (cur_file_stat.st_mode&0777);
   }

   if( strcmp(dest, "-") == 0 )
      dfd = 1;
   else
   {
      dfd = open(dest, O_WRONLY|O_TRUNC|O_CREAT, mmode);
      if(dfd<0)
      {
         close(sfd);
         return error(errno, "Cannot create ", source);
      }
   }

   if( fstat(dfd, &st) )
   {
      retv = error(errno, "", dest);
      no_seek = 1;
   }
   else
   {
#ifndef __BCC__
      blksz = st.st_blksize;
#endif
      no_seek = !S_ISREG(st.st_mode);
   }
   buf = alloca(blksz + sizeof(int));
   if( buf == 0 ) return error(0, "Out of memory", "");

   for(;;)
   {
      cc = read(sfd, buf, blksz);
      if(cc<0)
      {
         retv = error(errno, "", source);
         goto exit_now;
      }
      if(cc==0) break;
      buf[cc] = 1;
      for(ptr=buf; *ptr==0 ; ptr++) ;
      if((hole_flag = (ptr == buf+cc)))
      { /* Make a hole */
         if( lseek(dfd, (off_t) cc, SEEK_CUR) < 0 )
	 {
	    retv = error(errno, "", dest);
	    goto exit_now;
	 }
      }
      else
      {
         if( cc != write(dfd, buf, cc))
	 {
	    retv = error(errno, "", dest);
	    goto exit_now;
	 }
      }
   }
   if( hole_flag )
   {
      if( lseek(dfd, (off_t) -1, SEEK_CUR) < 0
       || write(dfd, "", 1) != 1 )
      {
	 retv = error(errno, "", dest);
	 goto exit_now;
      }
   }

exit_now:
   if(sfd>2) close(sfd);
   if(dfd>2) close(dfd);
   return retv;
}

void
Usage()
{
   int i;

   printf("FileTool Usage: %s%s", prog_name[0]=='-'?"ft -":"", prog_name);
   if( cmd_tok == CMD_FT )
   {
      printf(" --[com_name] [-options] [files]\n");
      printf("\nAvailable commands are:\n");
   }

   for(i=1; command_list[i].name; i++)
   {
      if( cmd_tok == CMD_FT )
         printf(" %s --%s", prog_name, command_list[i].name);
      else if( cmd_tok != command_list[i].cmd )
         continue;

      if( *command_list[i].opts )
         printf(" [-%s]", command_list[i].opts);
      switch(command_list[i].argpat)
      {
      case 1:  printf(" <info> [files]"); break;
      case -1: printf(" [files] [dest]"); break;
      case 0:  printf(" [files]"); break;
      default: printf(" path [bcu] major minor"); break;
      }
      printf("\n");
   }

   exit(99);
}

int
cmd_mkdir(dirname)
char * dirname;
{
   int retv;
   int mode = 0777;
   if( set_mode >= 0 ) mode = set_mode;

   retv = mkdir(dirname, mode);
   if(retv<0)
   {
      if(flg_mkpdir && errno == ENOENT)
      {
         /* Create parents */
      }
   }
   if( retv>=0 && cmd_tok == CMD_MKDIR )
   {
      if( set_user != -1 || set_group != -1 )
      {
         if( chown(dirname, set_user, set_group) < 0)
	    warning(errno, "Cannot change directory owner ", dirname);
         else if( chmod (dirname, mode & 07777) )
            warning(errno, "", dirname);
      }
   }

   if(retv<0) error(errno, "Cannot create directory ", dirname);
   return retv;
}

int
cmd_mknod()
{
   int device, major, minor;
   int rv = -1;
   int mode=0666;
   if( set_mode >= 0 ) mode=set_mode;

   major = atoi(flist[2]);
   minor = atoi(flist[3]);
#ifdef __linux__
   /* Linux 2.6+ uses an odd arrangment. */
   device = (major<<8) + (minor & 0xFF) + ((minor & 0xFFF00) << 12);
#else
   device = (major<<8) + (minor & 0xFF);
#endif

   if(flist[1][0] == 'b')
      rv = mknod(flist[0], S_IFBLK|mode, device);
   else if(flist[1][0] == 'c' || flist[1][0] == 'u')
      rv = mknod(flist[0], S_IFCHR|mode, device);
   else Usage();

   if(rv<0)
   {
      error(errno, "", flist[0]);
      exit(1);
   }
   return rv;
}

int
warning(enumber, estr, eobj)
int enumber; char * estr; char * eobj;
{
   if(flg_verbose)
      return error(enumber, estr, eobj);
   return 0;
}

int
error(enumber, estr, eobj)
int enumber; char * estr; char * eobj;
{
   fprintf(stderr, "%s%s: ", prog_name[0]=='-'?"ft":"", prog_name);
   fprintf(stderr, "%s%s: %s\n", estr, eobj, strerror(enumber));
   return -1;
}
