#include <stdio.h>
#include <string.h>


int main() {
    char s1[32];
    char s2[32];
    
    sprintf(s1, "AB");
    sprintf(s2, "DEF");

    strlcat(s1, s2, 6);

    printf("'%s'\r\n", s1);

    return 0;
}

