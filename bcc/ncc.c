
#include <stdio.h>

/* Normal passes ... 
 * p: C preprocessor	.c -> .0
 * 1: cc1		.1 -> .2
 * 2: opt		.2 -> .3
 * 3: cg		.3 -> .s
 * a: as  		.s -> .o
 * l: ld  		.o -> .out
 *
 * c: Is alias for one or more of 0,1,2
 * d: This command. (Semi-colon seperators)
 */

#define MAXPASSES	16
#define MAXOPTIONS	32

struct pass {
   char * command;
   char pass_id;
   char source;
   char result;
   char * args;
} passes[MAXPASSES];

struct option {
   char * optname;
   char * subst;
} options[MAXOPTS];

/* The default argument list ... */
extern int     standard_argc;
extern char ** standard_argv;

#ifdef __MSDOS__
int standard_argc = 0;		/* Nothing ... use the files */
char ** standard_argv = 0;
#endif

struct { char * opt, *spec; } builtin_models[] = {
   { "-0", "i8086" }
   { "-3", "i386" }
   { "-9", "mc6809" }
   {0,0}
};

/* What sort of compile ? */
char model[20] = "-";
int model_found = 0;
int args_active = 1;

/* Linker output file */
char * output_file = "a.out";

char ** file_list = 0;
int file_count = 0;
int pass_count = 0;
int opts_count = 0;

/* A temp buffer */
char little_buf[128];

main(argc, argv)
int argc;
char ** argv;
{
   int ar;
   /* First, is there a model ? */
   /* PROBLEM: This doesn't find -b specifiers that are embedded in user opts */
   for(ar=1; ar<argc; ar++)
      if(argv[ar][0] == '-') switch(argv[ar][1])
      {
      case 'o': ar++; break;
      case 'b': if(argv[ar][2]) break;
                ar++;
	        if( argv[ar] == 0 ) fatal("-b needs specifier");
		strncpy(model, argv[ar], sizeof(model)-1);
		break;
      default: /* Scan builtin_models */
      }

   /* Space for filenames */
   file_list = calloc(argc, sizeof(char*));
   if( file_list == 0 ) fatal("Out of memory");

   /* Perhaps: make builtin_models[0].spec the default ? */

   for(ar=0; ar<standard_argc; )
      ar += decode_arg(standard_argv[ar], standard_argv[ar+1]);

   /* The model isn't in the standard args ... */
   if( !model_found )
   {
      strcat(little_buf, exec_paths[0]);
      strcat(little_buf, "/spec");
      include_file(little_buf);
   }
   if( !model_found && strcmp(model, "-") != 0 )
   {
      strcat(little_buf, exec_paths[0]);
      strcat(little_buf, "/spec.");
      strcat(little_buf, model);
      include_file(little_buf);
   }
   if( !model_found ) fatal("Unable to find compiler specification");

   /* Now we can do the command line for real */
   for(ar=1; ar<argc; )
      ar += decode_arg(argv[ar], argv[ar+1]);

   /* Compile all the files to objects; save the names back in file_table */
   for(ar=0; ar<file_count; ar++)
      do_compile(file_table[ar]);

   /* If everything was OK then find the pass with a nul result type */
   if( exit_status == 0 )
      do_link_command();

   return exit_status;
}

decode_arg(arg1, arg2)
char * arg1;
char * arg2;
{
   if(arg1[0] == '-' && arg1[2] == '\0') switch(arg1[1])
   {
   case 'v': verbose++; return 1;
   case 'o': if( arg2 == 0 ) fatal("-o needs filename");
	     output_file = arg2;
	     return 2;
   case 'b': if( arg2 == 0 ) fatal("-b needs specifier");
	     args_active = 1;
             if( strcmp(arg2, "-") == 0)
	     	return 2;
	     else if( strcmp(arg2, model) == 0)
	     	;
	     else if( strcmp(model, "-") == 0)
		strncpy(model, arg2, sizeof(model)-1);
	     else
	     {
	        args_active=0;
                return 2;
	     }
	     model_found = 1;
             return 2;

   default: /* Lookup item in conversion table */
   }
   else if( args_active == 0 )
      return 1;
   else if(arg1[0] == '-' ) switch(arg1[1])
   {
   case '@': include_file(arg1+2); 		return 1;
   case 'B': add_path(exec_path, arg1+2); 	return 1;
   case 'W': command_W(arg1+2); 		return 1;
   case 'T': command_T(arg1+2); 		return 1;
   	     /* -T0,bcc-cc1,-0,-i,$1,$2,-o,$3    ie add standard args */

   default: /* Lookup item in conversion table */
   }
   else
   {
      file_names[file_count++] = arg1;
      return 1;
   }

   /* Do conversion ... */
   /*
    * 1) Locate arg in options
    * 2) for each semicolon seperated item
    * 3) pass it to decode_arg
    * 4) remember to give it 2 args for '-b'
    *
    * Note: char '*' in the options table matches the rest of the arg.
    */
   return 1;
}

include_file(fname)
char * fname;
{
   FILE * fd;
   char line_buffer[512];
   char * p;

   RIGHT HERE

   fd = fopen(fname, "r");
   if( fd == 0 ) fatal("Cannot open command line file");

   while( fgets(line_buffer, sizeof(line_buffer), fd) )
   {
      if( line_buffer[0] == '#' ) continue;

   }
   fclose(fd);
}

command_T(arg1)
char * arg1;
{
   /* Is it an option specifier ? */
   if(arg1[0] == '-')
   {
      argend = strchr(arg1, ';');


   }
   else /* Must be a pass specifier or modifier */
   {
      
   }
}
