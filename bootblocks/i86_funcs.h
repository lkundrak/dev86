/*
 * These are bios and hardware functions for the 8086 IBM PC
 */

extern int   x86;		/* CPU major number (0-3) */
extern char *x86_name;		/* and it's name */
extern int   x86_emu;		/* Is this a PC emulator ? */
extern int   x86_a20_closed;	/* Is the A20 gate closed ? */
extern int   x86_fpu;
extern int   x86_test;		/* Running in test mode ? */

extern unsigned boot_mem_top;	/* Top of RAM below 1M */
extern long     main_mem_top;	/* Top of RAM above 1M */

int a20_closed();
void open_a20();
void bios_open_a20();
void cpu_check();
void mem_check();
int  ext_put();
int  ext_get();
