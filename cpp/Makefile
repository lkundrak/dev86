CFLAGS=-Wall -Wstrict-prototypes

all: bcc-cpp

bcc-cpp: main.o cpp.o hash.o token1.o token2.o
	$(CC) $(CFLAGS) -o bcc-cpp main.o cpp.o hash.o token1.o token2.o

realclean: clean
	rm -f bcc-cpp token1.h token2.h

clean:
	rm -f main.o cpp.o hash.o token1.o token2.o

main.o: cc.h
cpp.o: cc.h
hash.o: cc.h
tree.o: cc.h

token1.o: token1.h
token2.o: token2.h

token1.h: token1.tok
	gperf -aptTc -N is_ctok -H hash1 token1.tok > token1.h

token2.h: token2.tok
	gperf -aptTc -k1,3 -N is_ckey -H hash2 token2.tok > token2.h
