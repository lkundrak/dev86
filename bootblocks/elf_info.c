
#include <stdio.h>
#include "elf_info.h"

Elf32_Ehdr elf_head;
Elf32_Phdr *elf_prog;

#ifdef TEST
FILE *ifd;

main(argc, argv)
int   argc;
char **argv;
{
   ifd = fopen(argv[1], "r");
   if (ifd == 0)
      exit(1);

   read_elfprog();
   write_ram("", -1L, 0);

   fclose(ifd);
}

read_file(buf, offset, len)
void *buf;
long  offset;
int   len;
{
   fseek(ifd, offset, 0);
   return fread(buf, 1, len, ifd);
}

write_ram(buf, linear, len)
char *buf;
long  linear;
int   len;
{
static long llen = 0;
static long lastlin= -1;

   if( llen > 0 && lastlin + llen != linear )
   {
      printf("for %8ld bytes\n", llen);
      lastlin= -1;
   }
   if( lastlin == -1 )
   {
      lastlin = linear;
      llen = 0;
   
      if( linear != -1 )
         printf("Write %08lx ", linear);
   }
   llen += len;
   return len;
}

error(str)
char *str;
{
   printf("Error: %s\n", str);
   return -1;
}
#endif

info_elf()
{
   int   i;

   printf("ELF-386 executable, entry = 0x%08lx\n", elf_head.e_entry);
   printf("\t\toffset   paddr    vaddr    filesz   memsz    align\n");
   for (i = 0; i < elf_head.e_phnum; i++)
   {
      printf(" %d: ", i);
      switch ((int) elf_prog[i].p_type)
      {
      case PT_NULL:
	 printf("PT_NULL");
	 break;
      case PT_LOAD:
	 printf("PT_LOAD");
	 break;
      case PT_DYNAMIC:
	 printf("PT_DYNAMIC");
	 break;
      case PT_INTERP:
	 printf("PT_INTERP");
	 break;
      case PT_NOTE:
	 printf("PT_NOTE");
	 break;
      case PT_SHLIB:
	 printf("PT_SHLIB");
	 break;
      case PT_PHDR:
	 printf("PT_PHDR");
	 break;
      default:
	 printf("PT_(%d)", elf_prog[i].p_type);
	 break;
      }
      printf("\t%08lx %08lx %08lx %08lx %08lx %08lx",
	     elf_prog[i].p_offset,
	     elf_prog[i].p_paddr,
	     elf_prog[i].p_vaddr,
	     elf_prog[i].p_filesz,
	     elf_prog[i].p_memsz,
	     elf_prog[i].p_align
	  );
      printf("\n");
   }
}

read_elfprog()
{
   static unsigned char elf_ok[] =
   {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3, ELFCLASS32, ELFDATA2LSB, EV_CURRENT};

   int   i;
   char  page_buf[4096];

   if (read_file(&elf_head, 0L, sizeof(elf_head)) != sizeof(elf_head))
      return error("Can't read ELF header");

   if (memcmp(elf_head.e_ident, elf_ok, 7) != 0 ||
       elf_head.e_type != ET_EXEC ||
       elf_head.e_machine != EM_386 ||
       elf_head.e_phnum <= 0 ||
       elf_head.e_phentsize != sizeof(Elf32_Phdr)
       )
      return error("Not a 386 executable ELF program");

   elf_prog = malloc(i = sizeof(Elf32_Phdr) * elf_head.e_phnum);
   if (elf_prog == 0)
      return error("Out of memory");

   if (read_file(elf_prog, elf_head.e_phoff, i) != i)
      return error("Can't read ELF program header");

   info_elf();

   for (i = 0; i < elf_head.e_phnum; i++)
   {
      long from, to, length, copied;
      int chunk;

      switch ((int) elf_prog[i].p_type)
      {
      case PT_NULL:
      case PT_NOTE:
	 continue;
      default:
	 return error("ELF: Unusable program segment (Must be static)");
      case PT_LOAD:
	 break;
      }
      from=elf_prog[i].p_offset;
      to=elf_prog[i].p_vaddr;
      length=elf_prog[i].p_filesz;

      for(copied=0; copied<length; )
      {
         if(length>copied+sizeof(page_buf)) chunk=sizeof(page_buf);
	 else chunk=length-copied;

	 if( (chunk=read_file(page_buf, from, chunk)) <= 0 )
	    return error("ELF Cannot read executable");
	 if( write_ram(page_buf, to, chunk) < 0 )
	    return error("Memory save failed");
         copied+=chunk; from+=chunk; to+=chunk;
      }
      length=elf_prog[i].p_memsz;
      if( length > copied )
      {
         write_ram("", -1L, 0);
	 memset(page_buf, '\0', sizeof(page_buf));
	 for(; copied<length; )
	 {
	    if(length>copied+sizeof(page_buf)) chunk=sizeof(page_buf);
	    else chunk=length-copied;

	    if( write_ram(page_buf, to, chunk) < 0 )
	       return error("Memory zap failed");
	    copied+=chunk; to+=chunk;
	 }
      }
   }
}
