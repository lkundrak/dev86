@echo off
if not exist later.exe cl -nologo -O later.c %LIB%\setargv.obj -link /NOE

later bcc/bcc.obj bcc/bcc.c
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -c -Fobcc\bcc.obj bcc\bcc.c
if errorlevel 1 goto exit_now
later bin/bcc.exe bcc/bcc.obj
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -o bin\bcc.exe bcc\bcc.obj %LIB%\setargv.obj -link /NOE
if errorlevel 1 goto exit_now

later bcc/bcc-cc1.obj bcc/bcc-cc1.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/bcc-cc1.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/bcc-cc1.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/bcc-cc1.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\bcc-cc1.obj bcc\bcc-cc1.c
if errorlevel 1 goto exit_now
later bcc/assign.obj bcc/assign.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/assign.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/assign.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/assign.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\assign.obj bcc\assign.c
if errorlevel 1 goto exit_now
later bcc/codefrag.obj bcc/codefrag.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/codefrag.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/codefrag.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/codefrag.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\codefrag.obj bcc\codefrag.c
if errorlevel 1 goto exit_now
later bcc/debug.obj bcc/debug.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/debug.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/debug.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/debug.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\debug.obj bcc\debug.c
if errorlevel 1 goto exit_now
later bcc/declare.obj bcc/declare.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/declare.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/declare.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/declare.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\declare.obj bcc\declare.c
if errorlevel 1 goto exit_now
later bcc/express.obj bcc/express.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/express.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/express.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/express.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\express.obj bcc\express.c
if errorlevel 1 goto exit_now
later bcc/exptree.obj bcc/exptree.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/exptree.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/exptree.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/exptree.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\exptree.obj bcc\exptree.c
if errorlevel 1 goto exit_now
later bcc/floatop.obj bcc/floatop.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/floatop.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/floatop.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/floatop.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\floatop.obj bcc\floatop.c
if errorlevel 1 goto exit_now
later bcc/function.obj bcc/function.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/function.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/function.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/function.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\function.obj bcc\function.c
if errorlevel 1 goto exit_now
later bcc/gencode.obj bcc/gencode.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/gencode.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/gencode.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/gencode.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\gencode.obj bcc\gencode.c
if errorlevel 1 goto exit_now
later bcc/genloads.obj bcc/genloads.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/genloads.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/genloads.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/genloads.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\genloads.obj bcc\genloads.c
if errorlevel 1 goto exit_now
later bcc/glogcode.obj bcc/glogcode.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/glogcode.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/glogcode.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/glogcode.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\glogcode.obj bcc\glogcode.c
if errorlevel 1 goto exit_now
later bcc/hardop.obj bcc/hardop.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/hardop.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/hardop.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/hardop.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\hardop.obj bcc\hardop.c
if errorlevel 1 goto exit_now
later bcc/input.obj bcc/input.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/input.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/input.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/input.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\input.obj bcc\input.c
if errorlevel 1 goto exit_now
later bcc/label.obj bcc/label.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/label.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/label.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/label.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\label.obj bcc\label.c
if errorlevel 1 goto exit_now
later bcc/loadexp.obj bcc/loadexp.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/loadexp.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/loadexp.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/loadexp.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\loadexp.obj bcc\loadexp.c
if errorlevel 1 goto exit_now
later bcc/longop.obj bcc/longop.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/longop.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/longop.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/longop.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\longop.obj bcc\longop.c
if errorlevel 1 goto exit_now
later bcc/output.obj bcc/output.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/output.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/output.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/output.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\output.obj bcc\output.c
if errorlevel 1 goto exit_now
later bcc/preproc.obj bcc/preproc.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/preproc.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/preproc.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/preproc.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\preproc.obj bcc\preproc.c
if errorlevel 1 goto exit_now
later bcc/preserve.obj bcc/preserve.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/preserve.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/preserve.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/preserve.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\preserve.obj bcc\preserve.c
if errorlevel 1 goto exit_now
later bcc/scan.obj bcc/scan.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/scan.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/scan.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/scan.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\scan.obj bcc\scan.c
if errorlevel 1 goto exit_now
later bcc/softop.obj bcc/softop.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/softop.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/softop.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/softop.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\softop.obj bcc\softop.c
if errorlevel 1 goto exit_now
later bcc/state.obj bcc/state.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/state.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/state.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/state.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\state.obj bcc\state.c
if errorlevel 1 goto exit_now
later bcc/table.obj bcc/table.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/table.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/table.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/table.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\table.obj bcc\table.c
if errorlevel 1 goto exit_now
later bcc/type.obj bcc/type.c bcc/align.h bcc/byteord.h bcc/condcode.h bcc/const.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/type.obj bcc/gencode.h bcc/input.h bcc/label.h bcc/os.h bcc/output.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/type.obj bcc/parse.h bcc/proto.h bcc/reg.h bcc/sc.h bcc/scan.h bcc/sizes.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later bcc/type.obj bcc/table.h bcc/type.h bcc/types.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fobcc\type.obj bcc\type.c
if errorlevel 1 goto exit_now
later lib/bcc-cc1.exe bcc/bcc-cc1.obj bcc/assign.obj bcc/codefrag.obj bcc/debug.obj
if errorlevel 3 goto exit_now
if not errorlevel 1 later lib/bcc-cc1.exe bcc/declare.obj bcc/express.obj bcc/exptree.obj bcc/floatop.obj
if errorlevel 3 goto exit_now
if not errorlevel 1 later lib/bcc-cc1.exe bcc/function.obj bcc/gencode.obj bcc/genloads.obj bcc/glogcode.obj
if errorlevel 3 goto exit_now
if not errorlevel 1 later lib/bcc-cc1.exe bcc/hardop.obj bcc/input.obj bcc/label.obj bcc/loadexp.obj
if errorlevel 3 goto exit_now
if not errorlevel 1 later lib/bcc-cc1.exe bcc/longop.obj bcc/output.obj bcc/preproc.obj bcc/preserve.obj
if errorlevel 3 goto exit_now
if not errorlevel 1 later lib/bcc-cc1.exe bcc/scan.obj bcc/softop.obj bcc/state.obj bcc/table.obj bcc/type.obj
if errorlevel 3 goto exit_now
if not errorlevel 1 goto done_bcc-cc1
if exist doslib.lib del doslib.lib
lib doslib.lib +bcc\assign.obj +bcc\codefrag.obj +bcc\debug.obj; >NUL
lib doslib.lib +bcc\declare.obj +bcc\express.obj +bcc\exptree.obj; >NUL
lib doslib.lib +bcc\floatop.obj +bcc\function.obj +bcc\gencode.obj; >NUL
lib doslib.lib +bcc\genloads.obj +bcc\glogcode.obj +bcc\hardop.obj; >NUL
lib doslib.lib +bcc\input.obj +bcc\label.obj +bcc\loadexp.obj +bcc\longop.obj; >NUL
lib doslib.lib +bcc\output.obj +bcc\preproc.obj +bcc\preserve.obj; >NUL
lib doslib.lib +bcc\scan.obj +bcc\softop.obj +bcc\state.obj +bcc\table.obj; >NUL
lib doslib.lib +bcc\type.obj; >NUL
cl -Ml -o lib\bcc-cc1.exe bcc\bcc-cc1.obj doslib.lib 
if errorlevel 1 goto exit_now
if exist doslib.lib del doslib.lib
if exist doslib.bak del doslib.bak
:done_bcc-cc1

later as/as.obj as/as.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/as.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/as.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\as.obj as\as.c
if errorlevel 1 goto exit_now
later as/assemble.obj as/assemble.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/assemble.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/assemble.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\assemble.obj as\assemble.c
if errorlevel 1 goto exit_now
later as/error.obj as/error.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/error.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/error.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\error.obj as\error.c
if errorlevel 1 goto exit_now
later as/express.obj as/express.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/express.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/express.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\express.obj as\express.c
if errorlevel 1 goto exit_now
later as/genbin.obj as/genbin.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/genbin.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/genbin.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\genbin.obj as\genbin.c
if errorlevel 1 goto exit_now
later as/genlist.obj as/genlist.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/genlist.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/genlist.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\genlist.obj as\genlist.c
if errorlevel 1 goto exit_now
later as/genobj.obj as/genobj.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/genobj.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/genobj.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\genobj.obj as\genobj.c
if errorlevel 1 goto exit_now
later as/gensym.obj as/gensym.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/gensym.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/gensym.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\gensym.obj as\gensym.c
if errorlevel 1 goto exit_now
later as/keywords.obj as/keywords.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/keywords.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/keywords.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\keywords.obj as\keywords.c
if errorlevel 1 goto exit_now
later as/macro.obj as/macro.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/macro.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/macro.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\macro.obj as\macro.c
if errorlevel 1 goto exit_now
later as/mops.obj as/mops.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/mops.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/mops.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\mops.obj as\mops.c
if errorlevel 1 goto exit_now
later as/pops.obj as/pops.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/pops.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/pops.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\pops.obj as\pops.c
if errorlevel 1 goto exit_now
later as/readsrc.obj as/readsrc.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/readsrc.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/readsrc.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\readsrc.obj as\readsrc.c
if errorlevel 1 goto exit_now
later as/scan.obj as/scan.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/scan.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/scan.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\scan.obj as\scan.c
if errorlevel 1 goto exit_now
later as/table.obj as/table.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/table.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/table.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\table.obj as\table.c
if errorlevel 1 goto exit_now
later as/typeconv.obj as/typeconv.c as/address.h as/byteord.h as/const.h as/file.h as/flag.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/typeconv.obj as/globvar.h as/macro.h as/opcode.h as/proto.h as/scan.h as/source.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later as/typeconv.obj as/syshead.h as/type.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Foas\typeconv.obj as\typeconv.c
if errorlevel 1 goto exit_now
later bin/as.exe as/as.obj as/assemble.obj as/error.obj as/express.obj as/genbin.obj
if errorlevel 3 goto exit_now
if not errorlevel 1 later bin/as.exe as/genlist.obj as/genobj.obj as/gensym.obj as/keywords.obj
if errorlevel 3 goto exit_now
if not errorlevel 1 later bin/as.exe as/macro.obj as/mops.obj as/pops.obj as/readsrc.obj as/scan.obj
if errorlevel 3 goto exit_now
if not errorlevel 1 later bin/as.exe as/table.obj as/typeconv.obj
if errorlevel 3 goto exit_now
if not errorlevel 1 goto done_as
if exist doslib.lib del doslib.lib
lib doslib.lib +as\assemble.obj +as\error.obj +as\express.obj +as\genbin.obj; >NUL
lib doslib.lib +as\genlist.obj +as\genobj.obj +as\gensym.obj +as\keywords.obj; >NUL
lib doslib.lib +as\macro.obj +as\mops.obj +as\pops.obj +as\readsrc.obj; >NUL
lib doslib.lib +as\scan.obj +as\table.obj +as\typeconv.obj; >NUL
cl -Ml -o bin\as.exe as\as.obj doslib.lib 
if errorlevel 1 goto exit_now
if exist doslib.lib del doslib.lib
if exist doslib.bak del doslib.bak
:done_as

later ld/ld.obj ld/ld.c ld/align.h ld/ar.h ld/bindef.h ld/byteord.h ld/config.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later ld/ld.obj ld/const.h ld/globvar.h ld/obj.h ld/syshead.h ld/type.h ld/x86_aout.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fold\ld.obj ld\ld.c
if errorlevel 1 goto exit_now
later ld/dumps.obj ld/dumps.c ld/align.h ld/ar.h ld/bindef.h ld/byteord.h ld/config.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later ld/dumps.obj ld/const.h ld/globvar.h ld/obj.h ld/syshead.h ld/type.h ld/x86_aout.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fold\dumps.obj ld\dumps.c
if errorlevel 1 goto exit_now
later ld/io.obj ld/io.c ld/align.h ld/ar.h ld/bindef.h ld/byteord.h ld/config.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later ld/io.obj ld/const.h ld/globvar.h ld/obj.h ld/syshead.h ld/type.h ld/x86_aout.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fold\io.obj ld\io.c
if errorlevel 1 goto exit_now
later ld/linksyms.obj ld/linksyms.c ld/align.h ld/ar.h ld/bindef.h ld/byteord.h ld/config.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later ld/linksyms.obj ld/const.h ld/globvar.h ld/obj.h ld/syshead.h ld/type.h ld/x86_aout.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fold\linksyms.obj ld\linksyms.c
if errorlevel 1 goto exit_now
later ld/readobj.obj ld/readobj.c ld/align.h ld/ar.h ld/bindef.h ld/byteord.h ld/config.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later ld/readobj.obj ld/const.h ld/globvar.h ld/obj.h ld/syshead.h ld/type.h ld/x86_aout.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fold\readobj.obj ld\readobj.c
if errorlevel 1 goto exit_now
later ld/table.obj ld/table.c ld/align.h ld/ar.h ld/bindef.h ld/byteord.h ld/config.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later ld/table.obj ld/const.h ld/globvar.h ld/obj.h ld/syshead.h ld/type.h ld/x86_aout.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fold\table.obj ld\table.c
if errorlevel 1 goto exit_now
later ld/typeconv.obj ld/typeconv.c ld/align.h ld/ar.h ld/bindef.h ld/byteord.h ld/config.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later ld/typeconv.obj ld/const.h ld/globvar.h ld/obj.h ld/syshead.h ld/type.h ld/x86_aout.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fold\typeconv.obj ld\typeconv.c
if errorlevel 1 goto exit_now
later ld/writebin.obj ld/writebin.c ld/align.h ld/ar.h ld/bindef.h ld/byteord.h ld/config.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later ld/writebin.obj ld/const.h ld/globvar.h ld/obj.h ld/syshead.h ld/type.h ld/x86_aout.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fold\writebin.obj ld\writebin.c
if errorlevel 1 goto exit_now
later ld/writex86.obj ld/writex86.c ld/align.h ld/ar.h ld/bindef.h ld/byteord.h ld/config.h
if errorlevel 3 goto exit_now
if not errorlevel 1 later ld/writex86.obj ld/const.h ld/globvar.h ld/obj.h ld/syshead.h ld/type.h ld/x86_aout.h
if errorlevel 3 goto exit_now
if errorlevel 1 cl -Ml -nologo -O -DPOSIX_HEADERS_MISSING -c -Fold\writex86.obj ld\writex86.c
if errorlevel 1 goto exit_now
later lib/ld.exe ld/ld.obj ld/dumps.obj ld/io.obj ld/linksyms.obj ld/readobj.obj
if errorlevel 3 goto exit_now
if not errorlevel 1 later lib/ld.exe ld/table.obj ld/typeconv.obj ld/writebin.obj ld/writex86.obj
if errorlevel 3 goto exit_now
if not errorlevel 1 goto done_ld
if exist doslib.lib del doslib.lib
lib doslib.lib +ld\dumps.obj +ld\io.obj +ld\linksyms.obj +ld\readobj.obj; >NUL
lib doslib.lib +ld\table.obj +ld\typeconv.obj +ld\writebin.obj; >NUL
lib doslib.lib +ld\writex86.obj; >NUL
cl -Ml -o lib\ld.exe ld\ld.obj doslib.lib 
if errorlevel 1 goto exit_now
if exist doslib.lib del doslib.lib
if exist doslib.bak del doslib.bak
:done_ld

:exit_now
