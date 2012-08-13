
! Now make sure this isn't too big!
end_of_code:
  if *>table_start
   fail! Partition table overlaps
  endif

!-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
! Clear the sector to the bottom of the partition table.
 if *<table_start-1
  org table_start-1
  .byte 0
 endif
