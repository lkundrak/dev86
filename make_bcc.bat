@echo off

cd bcc
set CFLAGS=-O -nologo
cl %CFLAGS% -c bcc.c
cl %CFLAGS% -o bcc.exe bcc.obj %LIB%\setargv.obj -link /NOE

set CFLAGS=-Ml -nologo -W3
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

del bcc_lib.lib
lib bcc_lib.lib -+assign.obj -+declare.obj -+gencode.obj -+label.obj ;
lib bcc_lib.lib -+preserve.obj -+type.obj -+express.obj -+genloads.obj ;
lib bcc_lib.lib -+loadexp.obj -+scan.obj -+exptree.obj -+glogcode.obj ;
lib bcc_lib.lib -+longop.obj -+softop.obj -+codefrag.obj -+floatop.obj ;
lib bcc_lib.lib -+hardop.obj -+output.obj -+state.obj -+debug.obj ;
lib bcc_lib.lib -+function.obj -+input.obj -+preproc.obj -+table.obj ;

cl %CFLAGS% -o bcc-cc1.exe bcc-cc1.obj bcc_lib.lib
cd ..

:no_bcc
cd ld
set CFLAGS=-O -Ml -DPOSIX_HEADERS_MISSING -nologo

cl -c %CFLAGS% writebin.c
cl -c %CFLAGS% dumps.c
cl -c %CFLAGS% io.c
cl -c %CFLAGS% ld.c
cl -c %CFLAGS% readobj.c
cl -c %CFLAGS% table.c
cl -c %CFLAGS% typeconv.c
cl -O -nologo -o ld.exe dumps io ld readobj table typeconv writebin
cd ..

cd as
set CFLAGS=-O -Ml -DPOSIX_HEADERS_MISSING -nologo
del as.lib
lib as.lib -+..\ld\typeconv.obj;
cl -c %CFLAGS% as.c
cl -c %CFLAGS% assemble.c
cl -c %CFLAGS% error.c
cl -c %CFLAGS% express.c
cl -c %CFLAGS% genbin.c
cl -c %CFLAGS% genlist.c
cl -c %CFLAGS% genobj.c
cl -c %CFLAGS% gensym.c
cl -c %CFLAGS% macro.c
cl -c %CFLAGS% mops.c
cl -c %CFLAGS% pops.c
cl -c %CFLAGS% readsrc.c
cl -c %CFLAGS% scan.c
cl -c %CFLAGS% table.c
lib as.lib -+assemble.obj;
lib as.lib -+error.obj;
lib as.lib -+express.obj;
lib as.lib -+genbin.obj;
lib as.lib -+genlist.obj;
lib as.lib -+genobj.obj;
lib as.lib -+gensym.obj;
lib as.lib -+macro.obj;
lib as.lib -+mops.obj;
lib as.lib -+pops.obj;
lib as.lib -+readsrc.obj;
lib as.lib -+scan.obj;
lib as.lib -+table.obj;
cl %CFLAGS% -o as.exe as.obj as.lib
cd ..

copy bcc\bcc.exe bin\bcc.exe
copy bcc\bcc-cc1.exe lib\bcc-cc1.exe
copy as\as.exe lib\as86.exe
copy ld\ld.exe lib\ld86.exe

