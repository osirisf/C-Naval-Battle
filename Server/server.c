#include "../Utilities/utility.h"
#include "user.h"
user* head = NULL;
int counter = 0;
int listener;
fd_set master; 

void handle_log_in(int socket);
void handle_disconnect(int socket,fd_set* mstr);
void handle_who(int socket,int count);
void handle_conn_req(int socket);
void handle_conn_accept(int socket);
void handle_conn_refuse(int socket);
void handle_ready(int socket);
void handle_match_end(int socket);
void demux_command(client_request cmd,int socket) {
  printf("[LOG] Command: %d\n",cmd);
	switch(cmd) {
		case LOG_IN: 
			handle_log_in(socket);
			break;
		case WHO:
			handle_who(socket,counter);
			break;
    case CONN_REQ:
      handle_conn_req(socket);
      break;
    case CONN_ACK:
      printf("ACKING CONN\n");
      handle_conn_accept(socket);
      break;
    case CONN_REJ:
      handle_conn_refuse(socket);
      break;
    case READY:
      handle_ready(socket);
      break;
    case MATCH_END:
      handle_match_end(socket);
      break;
    case QUIT:  
      handle_disconnect(socket,&master);
      break;
    default: printf("[LOG] No action for: %d\n",cmd);
	}

	//print_list(head);
}

int main(int argc,char* argv[]) {
	if(argc < 2)
        exit(1);
    int porta = atoi(argv[1]);
    sockaddr_in listenerAddress;
    setupAddress(&listenerAddress,porta,NULL);
    listener = setupServerSocket(&listenerAddress);
    printf("[SERVER] Waiting for connections\n");

    FD_ZERO(&master);
    fd_set read_ready; FD_ZERO(&read_ready);
    int fdmax = listener;
    int fdescriptor;
    int tmpdescriptor;
    FD_SET(listener,&master);
   	while(true) {
   		read_ready = master;
   		select(fdmax+1,&read_ready,NULL,NULL,NULL);
   		for(fdescriptor = 0; fdescriptor <= fdmax; ++fdescriptor) {
   			if(FD_ISSET(fdescriptor,&read_ready)) {
   				if(fdescriptor == listener) {
   					//accept client request
   					printf("[SERVER] Serving client request\n");
   					sockaddr_in* client_address = malloc(sizeof(sockaddr_in));
   					memset(client_address,0,sizeof(*client_address));
   					unsigned int client_address_dim = sizeof(*client_address);
   					tmpdescriptor = accept(listener,(sockaddr*)client_address,&client_address_dim);
   					if(tmpdescriptor < 0) {
   						perror("accept");
   						continue;
   					}
   					FD_SET(tmpdescriptor,&master);
                    fdmax = (tmpdescriptor > fdmax)? tmpdescriptor:fdmax;
                    continue;
   				}
   				
   				boolean res; client_request msg;
   				//receive command from client
   				res = recv_int(fdescriptor,NULL,(int*)&msg);
   				if(res == 0) {
   					handle_disconnect(fdescriptor,&master);
   				} else {
   					demux_command(msg,fdescriptor);
   				}
   			}
   		}
   	}
   	close(listener);
}