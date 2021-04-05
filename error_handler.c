// Mateusz Szałowski 298976

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "error_handler.h"

void handle_error(char* message) {

    fprintf(stderr, "%s: %s\n", message, strerror(errno));
    exit(EXIT_FAILURE);

}
