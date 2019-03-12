#include <stdio.h>
#include <string.h>


int main() {
    char s1[32];
    char s2[32];
    char s3[32];

    sprintf(s2, "1234567890");
    strlcpy(s1, s2, 4);
    strlcpy(s3, s2, strlen(s2));

    printf("s1='%s'\r\ns2='%s'\r\ns3='%s'\r\n", s1, s2, s3);


    return 0;
}


