
#include <signal.h>

__const char * __const sys_siglist[NSIG] = 
{
   "Unknown signal",
   "Hangup",
   "Interrupt",
   "Quit",
   "Illegal instruction",
   "Trace/breakpoint trap",
   "IOT trap/Abort",
   "Bus error",
   "Floating point exception",
   "Killed",
   "User defined signal 1",
   "Segmentation fault",
   "User defined signal 2",
   "Broken pipe",
   "Alarm clock",
   "Terminated",
   "Stack fault",
   "Child exited",
   "Continued",
   "Stopped (signal)",
   "Stopped",
   "Stopped (tty input)",
   "Stopped (tty output)",
   "Possible I/O",
   "CPU time limit exceeded",
   "File size limit exceeded",
   "Virtual time alarm",
   "Profile signal",
   "Window size changed",
   "File lock lost",
   "Power failure",
   "Unused signal"
};
