
#define PERM_GETINFO   0x100
#define PERM_GETGROUP  0x200

struct {
   int pid;
   int ppid;
   int uid;
   int gid;
   int euid;
   int egid;
}
   __info_safe;

getgroups(count, locn)
int count;
void * locn;
{
   if( count < 0 ) {errno = EINVAL; return -1; }
   return __permissions(PERM_GETGROUP, count, locn);
}

getpid()
{
   __permissions(PERM_GETINFO, 6, &__info_safe);
   return __info_safe.pid;
}

getppid()
{
   return __permissions(PERM_GETITEM(1) /*, 0, 0 */);
}
