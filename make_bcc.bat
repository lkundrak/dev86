@
@set OPROMPT=%PROMPT%
@set PROMPT=$g
@goto mkbcc
:endbcc
@goto mkas
:endas
@goto mkld
:endld
@goto docopy
@
:mkbcc
@
cd bcc
set CFLAGS=-O -nologo
cl %CFLAGS% -c bcc.c
cl %CFLAGS% -o bcc.exe bcc.obj %LIB%\setargv.obj -link /NOE
@
set CFLAGS=-Ml -nologo
cl %CFLAGS% -c assign.c
cl %CFLAGS% -c bcc-cc1.c
cl %CFLAGS% -c codefrag.c
cl %CFLAGS% -c debug.c
cl %CFLAGS% -c declare.c
cl %CFLAGS% -c express.c
cl %CFLAGS% -c exptree.c
cl %CFLAGS% -c floatop.c
cl %CFLAGS% -c function.c
cl %CFLAGS% -c gencode.c
cl %CFLAGS% -c genloads.c
cl %CFLAGS% -c glogcode.c
cl %CFLAGS% -c hardop.c
cl %CFLAGS% -c input.c
cl %CFLAGS% -c label.c
cl %CFLAGS% -c loadexp.c
cl %CFLAGS% -c longop.c
cl %CFLAGS% -c output.c
cl %CFLAGS% -c preproc.c
cl %CFLAGS% -c preserve.c
cl %CFLAGS% -c scan.c
cl %CFLAGS% -c softop.c
cl %CFLAGS% -c state.c
cl %CFLAGS% -c table.c
cl %CFLAGS% -c type.c
@
del \tmp\bcc_lib.lib
lib \tmp\bcc_lib.lib +assign.obj +declare.obj +gencode.obj +label.obj ;
lib \tmp\bcc_lib.lib +preserve.obj +type.obj +express.obj +genloads.obj ;
lib \tmp\bcc_lib.lib +loadexp.obj +scan.obj +exptree.obj +glogcode.obj ;
lib \tmp\bcc_lib.lib +longop.obj +softop.obj +codefrag.obj +floatop.obj ;
lib \tmp\bcc_lib.lib +hardop.obj +output.obj +state.obj +debug.obj ;
lib \tmp\bcc_lib.lib +function.obj +input.obj +preproc.obj +table.obj ;
@
cl %CFLAGS% -o bcc-cc1.exe bcc-cc1.obj \tmp\bcc_lib.lib
del \tmp\bcc_lib.lib
del \tmp\bcc_lib.bak
cd ..
@
@goto endbcc
@
:mkld
@
cd ld
set CFLAGS=-O -Ml -DPOSIX_HEADERS_MISSING -nologo
@
cl -c %CFLAGS% writex86.c
cl -c %CFLAGS% linksyms.c
cl -c %CFLAGS% dumps.c
cl -c %CFLAGS% io.c
cl -c %CFLAGS% ld.c
cl -c %CFLAGS% readobj.c
cl -c %CFLAGS% table.c
cl -c %CFLAGS% typeconv.c
cl -O -nologo -o ld.exe dumps io ld readobj table typeconv linksyms writex86
cd ..
@
@goto endld
@
:mkas
@
cd as
set CFLAGS=-O -Ml -DPOSIX_HEADERS_MISSING -nologo
cl -c %CFLAGS% readsrc.c
cl -c %CFLAGS% as.c
cl -c %CFLAGS% assemble.c
cl -c %CFLAGS% error.c
cl -c %CFLAGS% express.c
cl -c %CFLAGS% genbin.c
cl -c %CFLAGS% genlist.c
cl -c %CFLAGS% genobj.c
cl -c %CFLAGS% gensym.c
cl -c %CFLAGS% keywords.c
cl -c %CFLAGS% macro.c
cl -c %CFLAGS% mops.c
cl -c %CFLAGS% pops.c
cl -c %CFLAGS% scan.c
cl -c %CFLAGS% table.c
cl -c %CFLAGS% typeconv.c
@
del \tmp\as.lib
lib \tmp\as.lib +assemble.obj +error.obj +express.obj +genbin.obj;
lib \tmp\as.lib +genlist.obj +genobj.obj +gensym.obj +keywords.obj;
lib \tmp\as.lib +macro.obj +mops.obj +pops.obj +readsrc.obj;
lib \tmp\as.lib +scan.obj +table.obj +typeconv.obj;
@
cl %CFLAGS% -o as.exe as.obj \tmp\as.lib
del \tmp\as.lib
del \tmp\as.bak
cd ..
@
@goto endas
@
:docopy
@
copy bcc\bcc.exe bin\bcc.exe
copy bcc\bcc-cc1.exe lib\bcc-cc1.exe
copy as\as.exe lib\as86.exe
copy ld\ld.exe lib\ld86.exe
@
:endoffile
@
@set PROMPT=%OPROMPT%
@set OPROMPT=
