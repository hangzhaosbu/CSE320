#include <stdlib.h>
#include <string.h>

#include "client_registry.h"
#include "exchange.h"
#include "trader.h"
#include "debug.h"
#include "csapp.h"
#include "server.h"

extern EXCHANGE *exchange;
extern CLIENT_REGISTRY *client_registry;

static void terminate(int status);

/*
 * "Bourse" exchange server.
 *
 * Usage: bourse <port>
 */

void handler(int num){
    terminate(EXIT_SUCCESS);
}

int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    int p_flag = 0;
    int count = 1;
    while(count < argc){

        if(strcmp(argv[count], "-p") == 0){
            p_flag = 1;
            break;
        }
        count++;
    }

    if(p_flag == 0 || (p_flag == 1 && count == argc)){
        fprintf(stderr,"Invalid format!\n");
        terminate(EXIT_FAILURE);
    }

    if(argc != 3){
        fprintf(stderr,"Invalid format!\n");
        terminate(EXIT_FAILURE);

    }

    // Perform required initializations of the client_registry,
    // maze, and player modules.
    client_registry = creg_init();
    exchange = exchange_init();
    trader_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function brs_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.
    struct sigaction action, old_action;

    sigset_t block_mask;
    sigemptyset(&block_mask);

    action.sa_handler = handler;
    action.sa_mask = block_mask;
    action.sa_flags = 0;


    if (sigaction(SIGHUP, &action, &old_action) < 0)
        terminate(EXIT_FAILURE);

    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
//    char *p = argv[count+1];
    listenfd = open_listenfd(argv[count+1]);
    pthread_t tid;
    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = malloc(sizeof(int));
        memset(connfdp, 0, sizeof(int));
        *connfdp = accept(listenfd, (SA *)&clientaddr, &clientlen);
        if(*connfdp < 0){
            free(connfdp);
            terminate(EXIT_FAILURE);
        }
        pthread_create(&tid,NULL,brs_client_service,connfdp);
    }

    fprintf(stderr, "You have to finish implementing main() "
	    "before the Bourse server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);
    
    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    exchange_fini(exchange);
    trader_fini();

    debug("Bourse server terminating");
    exit(status);
}
