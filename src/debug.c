#include <stdio.h>
#include <stdlib.h>
#include "debug.h"

/**
 * Exit the program and print error message
 *
 * @param message Error message
 */
void die(const char *message)
{
    if(errno)
    {
        perror(message);
    }
    else
    {
        printf("ERROR: %s\n", message);
    }

    exit(1);
}
