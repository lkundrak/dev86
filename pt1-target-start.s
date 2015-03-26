; ProgTool revision 1
; Startup code

	.extern _main
	.define _entry

	.text
	
; Jump to program main entry point

_entry:

	br _main
	
	end
