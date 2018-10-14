#include <stdio.h>
#include <string.h>



int main(int argc, char *argv[]) {
    char *reverse, str[12];

    sprintf(str, "Test String");

    reverse = strdup(strrev(str));

    printf("'%s' '%s'\r\n", str, reverse);

    return 0;
}


