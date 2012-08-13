@echo off
cl -Ms -nologo -O -c -Fo%TMP%\bcc.obj bcc\bcc.c
if errorlevel 1 goto exit_now

cl -Ms -o bin\bcc.exe %TMP%\bcc.obj %LIB%\setargv.obj -link /NOE
if errorlevel 1 goto exit_now

cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\main.obj cpp\main.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\cpp.obj cpp\cpp.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\hash.obj cpp\hash.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\token1.obj cpp\token1.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\token2.obj cpp\token2.c
if errorlevel 1 goto exit_now

if exist %TMP%\doslib.lib del %TMP%\doslib.lib
lib %TMP%\doslib.lib +%TMP%\cpp.obj +%TMP%\hash.obj +%TMP%\token1.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\token2.obj; 
if errorlevel 1 goto exit_now

cl -Ml -o lib\bcc-cpp.exe %TMP%\main.obj %TMP%\doslib.lib 
if errorlevel 1 goto exit_now

cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\bcc-cc1.obj bcc\bcc-cc1.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\assign.obj bcc\assign.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\codefrag.obj bcc\codefrag.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\debug.obj bcc\debug.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\declare.obj bcc\declare.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\express.obj bcc\express.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\exptree.obj bcc\exptree.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\floatop.obj bcc\floatop.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\function.obj bcc\function.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\gencode.obj bcc\gencode.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\genloads.obj bcc\genloads.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\glogcode.obj bcc\glogcode.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\hardop.obj bcc\hardop.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\input.obj bcc\input.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\label.obj bcc\label.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\loadexp.obj bcc\loadexp.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\longop.obj bcc\longop.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\output.obj bcc\output.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\preproc.obj bcc\preproc.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\preserve.obj bcc\preserve.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\scan.obj bcc\scan.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\softop.obj bcc\softop.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\state.obj bcc\state.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\table.obj bcc\table.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\type.obj bcc\type.c
if errorlevel 1 goto exit_now

if exist %TMP%\doslib.lib del %TMP%\doslib.lib
lib %TMP%\doslib.lib +%TMP%\assign.obj +%TMP%\codefrag.obj +%TMP%\debug.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\declare.obj +%TMP%\express.obj +%TMP%\exptree.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\floatop.obj +%TMP%\function.obj +%TMP%\gencode.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\genloads.obj +%TMP%\glogcode.obj +%TMP%\hardop.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\input.obj +%TMP%\label.obj +%TMP%\loadexp.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\longop.obj +%TMP%\output.obj +%TMP%\preproc.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\preserve.obj +%TMP%\scan.obj +%TMP%\softop.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\state.obj +%TMP%\table.obj +%TMP%\type.obj; 
if errorlevel 1 goto exit_now

cl -Ml -o lib\bcc-cc1.exe %TMP%\bcc-cc1.obj %TMP%\doslib.lib 
if errorlevel 1 goto exit_now

cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\alloc.obj as\alloc.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\as.obj as\as.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\assemble.obj as\assemble.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\errors.obj as\errors.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\express.obj as\express.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\genbin.obj as\genbin.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\genlist.obj as\genlist.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\genobj.obj as\genobj.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\gensym.obj as\gensym.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\keywords.obj as\keywords.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\macro.obj as\macro.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\mops.obj as\mops.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\pops.obj as\pops.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\readsrc.obj as\readsrc.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\scan.obj as\scan.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\table.obj as\table.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\typeconv.obj as\typeconv.c
if errorlevel 1 goto exit_now

if exist %TMP%\doslib.lib del %TMP%\doslib.lib
lib %TMP%\doslib.lib +%TMP%\as.obj +%TMP%\assemble.obj +%TMP%\errors.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\express.obj +%TMP%\genbin.obj +%TMP%\genlist.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\genobj.obj +%TMP%\gensym.obj +%TMP%\keywords.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\macro.obj +%TMP%\mops.obj +%TMP%\pops.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\readsrc.obj +%TMP%\scan.obj +%TMP%\table.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\typeconv.obj; 
if errorlevel 1 goto exit_now

cl -Ml -o bin\as86.exe %TMP%\alloc.obj %TMP%\doslib.lib 
if errorlevel 1 goto exit_now

cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\ld.obj ld\ld.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\dumps.obj ld\dumps.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\io.obj ld\io.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\linksyms.obj ld\linksyms.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\readobj.obj ld\readobj.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\table.obj ld\table.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\typeconv.obj ld\typeconv.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\writebin.obj ld\writebin.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\writex86.obj ld\writex86.c
if errorlevel 1 goto exit_now

if exist %TMP%\doslib.lib del %TMP%\doslib.lib
lib %TMP%\doslib.lib +%TMP%\dumps.obj +%TMP%\io.obj +%TMP%\linksyms.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\readobj.obj +%TMP%\table.obj +%TMP%\typeconv.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\writebin.obj +%TMP%\writex86.obj; 
if errorlevel 1 goto exit_now

cl -Ml -o bin\ld86.exe %TMP%\ld.obj %TMP%\doslib.lib 
if errorlevel 1 goto exit_now

cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\unproto.obj unproto\unproto.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\error.obj unproto\error.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\hash.obj unproto\hash.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\strsave.obj unproto\strsave.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\symbol.obj unproto\symbol.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\tok_clas.obj unproto\tok_clas.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\tok_io.obj unproto\tok_io.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\tok_pool.obj unproto\tok_pool.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\unproto.obj unproto\unproto.c
if errorlevel 1 goto exit_now
cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fo%TMP%\vstring.obj unproto\vstring.c
if errorlevel 1 goto exit_now

if exist %TMP%\doslib.lib del %TMP%\doslib.lib
lib %TMP%\doslib.lib +%TMP%\error.obj +%TMP%\hash.obj +%TMP%\strsave.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\symbol.obj +%TMP%\tok_clas.obj +%TMP%\tok_io.obj; 
if errorlevel 1 goto exit_now
lib %TMP%\doslib.lib +%TMP%\tok_pool.obj +%TMP%\unproto.obj +%TMP%\vstring.obj; 
if errorlevel 1 goto exit_now

cl -Ml -o lib\unproto.exe %TMP%\unproto.obj %TMP%\doslib.lib 
if errorlevel 1 goto exit_now

echo Compile complete.
:exit_now
