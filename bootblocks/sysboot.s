
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
dos_sysid:	.blkb 8		! System ID
dos_sect:	.word 0		! Sector size
dos_clust:	.byte 0		! Cluster size
dos_resv:	.word 0		! Res-sector
dos_nfat:	.byte 0		! FAT count
dos_nroot:	.word 0		! Root dir entries
dos_maxsect:	.word 0		! Sector count (=0 if large FS)
dos_media:	.byte 0		! Media code
dos_fatlen:	.word 0		! FAT length
dos_spt:	.word 0		! Sect/Track
dos_heads:	.word 0		! Heads
dos_hidden:	.long 0		! Hidden sectors

! Here down is DOS 4+ and probably not needed for floppy boots.
floppy_temp:

dos4_maxsect:	.long 0		! Large FS sector count
dos4_phy_drive:	.byte 0		! Phys drive
.byte 0		! Reserved
.byte 0		! DOS 4
dos4_serial:	.long 0		! Serial number
dos4_label:	.blkb 11	! Disk Label (DOS 4+)
dos4_fattype:	.blkb 8		! FAT type

.blkb sysboot_start+0x3E-*
public codestart
codestart:
  jmp	codestart

! Partition table
public partition_1
public partition_2
public partition_3
public partition_4
public bootblock_magic

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

bootblock_magic:
.blkb sysboot_start+0x1FE-*
.word 0xAA55

