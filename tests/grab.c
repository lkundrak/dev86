
#include <stdio.h>
#include <malloc.h>

struct s
{
   struct s * n;
   char v[1];
};

#define M  ((unsigned)-1>>1)
#define V  (M^(M>>1))

main (argc,argv)
int argc;
char ** argv;
{
	struct s * ptr1 = 0;
	struct s * ptr2;
	struct s * ptr3;
	int i,sz;
	unsigned long total = 0;

	for(i=0, sz=256 ; i<32; i++, sz = ((sz << 1) | (sz & V)) & M)
	{
		ptr2 = (struct s *) malloc(sz-sizeof(int));
		printf("%2d(%8u)..%08lx..%ld\n",i,sz,(long)ptr2,(long)ptr2);
		if(ptr2==0) break;
		total+=sz;
		if(ptr1==0)
		{
			ptr1 = ptr3 = ptr2;
			ptr3->n = 0;
		}
		else
		{
			ptr3->n = ptr2;
			ptr3 = ptr2;
			ptr3->n = 0;
		}
	}
	for(sz>>=1; sz>3; )
	{
		ptr2 = (struct s *) malloc(sz-sizeof(int));
		if(ptr2==0) { sz >>=1; continue; }
		printf("%2d(%8u)..%08lx..%ld\n",i++,sz,(long)ptr2,(long)ptr2);
		total+=sz;
		if(ptr1==0)
		{
			ptr1 = ptr3 = ptr2;
			ptr3->n = 0;
		}
		else
		{
			ptr3->n = ptr2;
			ptr3 = ptr2;
			ptr3->n = 0;
		}
	}
	printf("Free all - total was %ldK bytes\n", total/1024);
	while( ptr1 )
	{
		ptr3 = ptr1->n;
		free(ptr1);
		ptr1 = ptr3;
	}
	ptr2 = (struct s *) malloc(200);
	printf("%2d(%8u)..%08lx..%ld\n",i++,200,(long)ptr2,(long)ptr2);
	ptr2 = (struct s *) malloc(30000);
	printf("%2d(%8u)..%08lx..%ld\n",i++,30000,(long)ptr2,(long)ptr2);
	ptr2 = (struct s *) malloc(20000);
	printf("%2d(%8u)..%08lx..%ld\n",i++,20000,(long)ptr2,(long)ptr2);
	sz = (256<<sizeof(int));
        do
        {
	   ptr2 = (struct s *) malloc(sz-sizeof(int));
	   printf("%2d(%8u)..%08lx..%ld\n",i++,sz,(long)ptr2,(long)ptr2);
        }
	while(ptr2 && i < 100);
	exit(0);
}
