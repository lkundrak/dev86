#!/bin/sh -
#
# This file is simply an example of what can be done using the new binary
# and symbol table output functions. As shown it can be used to produce
# a C file containing the encapsulated binary of the assembly, plus any
# public symbols in the source are accessable to the C program.
#
# Use it in a makefile:
#
# .s.v:
#	as86_encap $*.s $*.v $*_ $(AS86FLAGS)
#

[ $# -lt 2 ] && {
   echo "Usage: `basename $0` infile outfile prefix [as86 opts]" 1>&2
   exit 1
}

trap "rm -f _$$.* ; exit 99" 1 2 3 15 

LIBDIR='%%LIBDIR%%'	# Set by make install
BINDIR='%%BINDIR%%'	# Set by make install

# If the one set by install fails then try a couple of others.
[ -x "$LIBDIR/as86" ] || LIBDIR="`dirname $0`"
[ -x "$LIBDIR/as86" ] || LIBDIR="$BINDIR"
[ -x "$LIBDIR/as86" ] || LIBDIR="`dirname $0`/../lib"
[ -x "$LIBDIR/as86" ] || LIBDIR=/usr/bin

IFILE="$1"
OFILE="$2"
PREFIX="`basename $IFILE .s`_"

shift ; shift
if [ $# -ge 1 ]
then case "$1" in 
     -* ) ;; 
     [A-Za-z_]* ) PREFIX="$1"
                  shift
		  ;;
     esac
fi
RV=0

$LIBDIR/as86 "$@" "$IFILE" -b _$$.bin -s _$$.sym || RV=$?

[ "$RV" = 0 ] && {
  (
    sort _$$.sym
    echo %%%%
    od -v -t uC _$$.bin
  ) | \
  awk > _$$.v -v prefix=$PREFIX ' BEGIN{
       sname = prefix "start";
    }
    /^%%%%$/ { flg++; 
       if( flg == 1 )
       {
	  if( !started )
	     printf "#define %s 0\n", sname;

	  printf "\n";
	  printf "static char %sdata[] = {\n", prefix;
          bincount=0;
       }
       next;
    }
    flg==0 {
       if(NF == 0) next;
       if( substr($2,1,4) == "0000" ) $2=substr($2,5);
       if( $1 == "+" && $4 == "$start" )
       {
          printf "#define %s 0x%s\n", sname, $2;
	  started = 1;
       }
       else if( substr($3, 1, 1) == "E" && $4 != "start" && $4 != "size" && $4 != "data" )
       {
          printf "#define %s%s 0x%s\n", prefix, $4, $2;
       }
       next;
    }
    flg==1 {
        if(NF == 0) next;
	printf "   ";
	for(i=2;i<=NF;i++) {
	   if( $i >= 32 && $i < 127 && $i != 39 && $i != 92 )
	      printf("\047%c\047,", $i);
	   else
	      printf("%3d,", $i);
	   bincount++;
	}
	printf "\n";
    }
    END {
       printf "};\n\n";
       printf "#define %ssize %d\n", prefix, bincount;
    }
  '
  RV=$?
}

[ "$RV" = 0 ] && {
   if [ "X$OFILE" = "X-" ]
   then cat _$$.v
   else mv -f _$$.v "$OFILE" || RV=$?
   fi
}

rm -f _$$.*
exit $RV
