#include "002_proxy_parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define MAX_CLIENTS 10
#define MAX_BYTES 4096



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


int thread_fn(void *socketNew){
    sem_wait(&semaphore);
    int p;
    sem_getvalue(&semaphore, p); //if semaphore value is -1 then its in USE, otherwise we can use it(for +ve values)
    printf("semaphore value is %d\n", p);
    int *t = (int*) socketNew;
    int socket = *t;    //gettinng the value from the POINTER
    int bytes_send_client, len;  //BYTES and LEN of it sent by CLIENT during a HTTP request


    char *buffer = (char *)calloc(MAX_BYTES, sizeof(char));
    bzero(buffer, MAX_BYTES);
    bytes_send_client = recv(socket, buffer, MAX_BYTES, 0);

    while (bytes_send_client > 0)
    {
        len = strlen(buffer);
        if(strstr(buffer, "\r\n\r\n") == NULL){  //checking if the BUFFER contains some values or if its EOF
            bytes_send_client = recv(socket, buffer + len, MAX_BYTES - len, 0);  //receiving the available bytes on the socket
        }else{ //otherwise break the LOOP
            break;  
        }
    }


    //below creating a copy of BUFFER into a tempReq to search in the LRU cache
    char *tempReq = (char*)malloc(strlen(buffer)*sizeof(char)+1);
    for(int i = 0; i < strlen(buffer); i++){
        tempReq[i] = buffer[i];
    }

    struct cache_element *temp = find(tempReq);  //finding the 'tempReq' in the CACHE

    if(temp != NULL){ //till the temp ptr is NULL
        int size = temp->len/sizeof(char);  
        int pos = 0;
        char response[MAX_BYTES];
        while (pos < size)    //this loop will send the requested URL to the socket
        {
            bzero(response, MAX_BYTES);
            for(int i = 0; i < MAX_BYTES; i++){
                response[i] = temp->data[i];
                pos++;
            }
            
            send(socket, response, MAX_BYTES, 0);  //this will send the response bytes to the SOCKET if any bytes exits/remaining 
        }

        printf("data retrieved from cache\n");
        printf("%s\n\n", response);
        
    }else if(bytes_send_client > 0){
        len = strlen(buffer);
        ParsedRequest *request = ParsedRequest_create();

        if(ParsedRequest_parse(request, buffer, len) < 0){
            printf("parsing failed\n");
        }else{
            bzero(buffer, MAX_BYTES);
            if{}
        }
    }
    

    


}

int main(int argc, char* argv[]){
    int client_socketId, client_len;
    struct sockaddr_in server_addr, client_addr;
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
    if(proxy_socketId < 0){
        perror("failed to create a socket\n");
        exit(1);
    }

    //the below code helps us to check for the ADDRESS already in use ERRORS-
    int reuse = 1;
    if(setsockopt(proxy_socketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0){
        perror("setSockOpt failed\n");
    }
    
    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);   //assigning PORT to the PROXY
    server_addr.sin_addr.s_addr = INADDR_ANY; //allowing the server to accept connections from any network interface.

    // Binding the socket
	if( bind(proxy_socketId, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 ) //assigning a name to a socket
	{
		perror("Port is not available\n");
		exit(1);
	}
	printf("Binding on port: %d\n",port_number);

    int listen_status = listen(proxy_socketId, MAX_CLIENTS);

    if(listen_status < 0){
        perror("error in listening\n");
        exit(1);
    }

    int i = 0;
    int Connected_socketId[MAX_CLIENTS];
    // Infinite Loop for accepting connections
	while(1)
	{
		
		bzero((char*)&client_addr, sizeof(client_addr));			// Clears struct client_addr
		client_len = sizeof(client_addr); 

        // Accepting the connections
		client_socketId = accept(proxy_socketId, (struct sockaddr*)&client_addr,(socklen_t*)&client_len);	// Accepts connection
		if(client_socketId < 0)
		{
			fprintf(stderr, "Error in Accepting connection !\n");
			exit(1);
		}
		else{
			Connected_socketId[i] = client_socketId; // Storing accepted client into array
		}

		// Getting IP address and port number of client
		struct sockaddr_in* client_pt = (struct sockaddr_in*)&client_addr;
		struct in_addr ip_addr = client_pt->sin_addr;
		char str[INET_ADDRSTRLEN];										// INET_ADDRSTRLEN: Default ip address size
		inet_ntop( AF_INET, &ip_addr, str, INET_ADDRSTRLEN );
		printf("Client is connected with port number: %d and ip address: %s \n",ntohs(client_addr.sin_port), str);
		//printf("Socket values of index %d in main function is %d\n",i, client_socketId);
		pthread_create(&tid[i],NULL,thread_fn, (void*)&Connected_socketId[i]); // Creating a thread for each client accepted
		i++; 
	}
	close(proxy_socketId);									// Close socket
 	return 0;
    /*timestamp - 45:00*/
}
