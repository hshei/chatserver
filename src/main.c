#include <stdlib.h>
#include "server.h"

int main(void){
    server_s *server = NULL;
    server_init(&server);
    server_run(server);

    return EXIT_SUCCESS;
}