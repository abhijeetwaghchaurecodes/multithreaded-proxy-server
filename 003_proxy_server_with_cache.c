#include "002_proxy_parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>  //to handle and manage sockets
#include <netdb.h>
#include <arpa/inet.h> //to support address related queries
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#define MAX_CLIENTS 10



typedef struct cache_element cache_element;
struct cache_element{
    /* data */
    char* data;   //header data of the website
    int len;    //length of the PAYLOAD in bytes
    char* url;  //the url we need from the REQUEST
    time_t lru_time_track;   //time based LRU tracking (two methods - counter and time based)
    cache_element* next;  //to make a LINKED-LIST of the LRU cache
};

cache_element* find(char* url);
int add_cache_element(char* data, int size, char url);
void remove_cache_element();

int port_number = 8080;  //defining the port we at which the PROXY-SERVER will run
int proxy_socketId;  //each client will have their own SOCKETS
pthread_t tid[MAX_CLIENTS]; //in total there are 10 MAX possible clients
sem_t semaphore;
pthread_mutex_t lock;  
/*above MUTEX lock- now whenever we're using a SHARED RESOURCE such as the LRU cache in this case
    we need to use the MUTEX lock in order to avoid the multi-JAMMING at that shared resource
    LOCKS have only two values - 0 or 1
    SEMAPHORES have more than 2 values
*/

cache_element* head;
int cache_size;

int main(int argc, char* argv[]){
    int client_socketId, client_len;
    struct sockaddr server_addr, client_addr;
    sem_init(&semaphore,0, MAX_CLIENTS);
    pthread_mutex_init(&lock, NULL);

    if(argv == 2){
        port_number = atoi(argv[1]);
    }
    else{
        printf("too few arguments\n");
        exit(1);
    }

    printf("starting proxy server at port: %d\n", port_number);
    proxy_socketId = socket(AF_INET, SOCK_STREAM, 0);

    /*timestamp - 29:00*/
}
