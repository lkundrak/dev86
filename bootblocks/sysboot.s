
! The master boot sector will have setup a stack,
! this is normally at 0:7c00 down.
! DS, SS, CS and ES will all have value 0 so the execution address is 0:7c00
! On entry the register SI will be pointer to the partition entry that
! this sector was loaded from, DL is the drive.

! Also if it's a standard Master boot DH will be the head, CX will be the
! sector and cylinder, BX=7C00, AX=1, DI=7DFE, BP=SI. There's a reasonable
! chance that this isn't true though.

! The Master boot itself will have been loaded and run at $07c00
! The BIOS must have setup a stack because interrupts are enabled
! Little else can be assumed because DOS doesn`t assume anything either

sysboot_start:
j codestart

.blkb sysboot_start+3-*
public dosfs_stat
dosfs_stat:
.blkb 8		! System ID
.word 0		! Sector size
.byte 0		! Cluster size
.word 0		! Res-sector
.byte 0		! FAT count
.word 0		! Root dir entries
.word 0		! Sector count (=0 if large FS)
.byte 0		! Media code
.word 0		! FAT length
.word 0		! Sect/Track
.word 0		! Heads
.long 0		! Hidden sectors
! Here down is DOS 4+
.long 0		! Large FS sector count
.byte 0		! Phys drive
.byte 0		! Reserved
.byte 0		! DOS 4
.long 0		! Serial number
.blkb 11	! Disk Label (DOS 4+)
.blkb 8		! FAT type

.blkb sysboot_start+0x3E-*
public codestart
codestart:
  hlt

! Partition table
public partition_1
public partition_2
public partition_3
public partition_4

.blkb sysboot_start+0x1BE-*
partition_1:
.byte 0,0,0,0,0,0,0,0	! IN,SH,SS,ST,OS,EH,ES,ET
.long 0			! Linear position (0 based)
.long 0			! Linear length
.blkb sysboot_start+0x1CE-*
partition_2:
.byte 0,0,0,0,0,0,0,0	! IN,SH,SS,ST,OS,EH,ES,ET
.long 0			! Linear position (0 based)
.long 0			! Linear length
.blkb sysboot_start+0x1DE-*
partition_3:
.byte 0,0,0,0,0,0,0,0	! IN,SH,SS,ST,OS,EH,ES,ET
.long 0			! Linear position (0 based)
.long 0			! Linear length
.blkb sysboot_start+0x1EE-*
partition_4:
.byte 0,0,0,0,0,0,0,0	! IN,SH,SS,ST,OS,EH,ES,ET
.long 0			! Linear position (0 based)
.long 0			! Linear length

.blkb sysboot_start+0x1FE-*
.word 0xAA55

