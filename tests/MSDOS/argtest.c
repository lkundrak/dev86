#include <stdio.h>
#include <string.h>
#include <dos.h>


int main(int argc, char *argv[]) {
    int i;
    
    for(i=0; i<argc; i++) {
        printf("argc=%d, argv='%s'\r\n", i, argv[i]);
        
    }
    return 0x5a5a;

}
