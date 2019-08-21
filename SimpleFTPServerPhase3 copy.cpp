#include<iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include<vector>
using namespace std;

/* Print usage of program */
void usage(char *progname) {
  fprintf(stderr, "%s <portNum>\n",
	  progname);
} // End usage()


int main(int argc, char *argv[]){
	/* Get command-line arguments */
	if(argc != 2) { usage(argv[0]); exit(1); }
	int portNum=atoi(argv[1]);

	fd_set readfds;
  vector<int> client_sockets;

	int socket_fd;
	struct sockaddr_in my_addr,their_addr;
	socket_fd = socket(PF_INET,SOCK_STREAM,0);
	if(socket_fd < 0) { perror("socket failed"); exit(20); }


	my_addr.sin_family = AF_INET; 
	my_addr.sin_port = htons(portNum);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	memset(&(my_addr.sin_zero), '\0', 8);
	int b = bind(socket_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));
	if(b==-1) { perror("Bind failed"); exit(2); }
	else { cout<<"BindDone: "<<portNum<<endl; }


	int l=listen(socket_fd,10);
	if(l==-1) {perror("Listen failed"); exit(2);}
	else {cout<<"ListenDone: "<<portNum<<endl; }

  while(true){
    FD_ZERO(&readfds); 
    FD_ZERO(&writefds); 
    FD_SET(socket_fd, &readfds);  
    FD_SET(socket_fd, &readfds);  
    int max_socket_fd = socket_fd;
    for(int i = 0;i < client_sockets.size();i++){  
      if(client_sockets[i] > 0){
        FD_SET(client_sockets[i], &readfds);
        max_socket_fd = max(socket_fd,client_sockets[i]);
      }  
    }

    int activity = select(max_socket_fd+1,&readfds,&writefds,NULL,NULL);

    if(FD_ISSET(socket_fd, &readfds)){  
      int new_fd = accept(socket_fd,(struct sockaddr *)&my_addr, (socklen_t*)&my_addr);
      if(new_fd<0) {perror("Accept failed"); exit(2);} 

      cout<<"New connection : "<<new_fd<<"  "<<inet_ntoa(my_addr.sin_addr)<<":"<<ntohs(my_addr.sin_port)<<endl;  
      client_sockets.push_back(new_fd);
    }

    for (int i = 0; i < client_sockets.size(); i++){
      if (FD_ISSET(client_sockets[i],&readfds)){  

        int bytesReceived = 0;
        char recvBuff[1024] = {0};
        memset(recvBuff, '0', sizeof(recvBuff));

        bytesReceived = read(client_sockets[i], recvBuff, 1024);

        if(bytesReceived==0){
          close(client_sockets[i]);  
          client_sockets[i] = 0;              
        }

        else{
          recvBuff[bytesReceived]='\0';

          char* fileToTransfer = recvBuff+4;

          cout<<"FileRequested: "<<fileToTransfer<<endl;

          if(access(fileToTransfer,R_OK) == -1 ) {cout<<"FileTransferFail\n"; perror("File not present or not readable"); close(client_sockets[i]);client_sockets[i] = 0;break;}
          int totalbytesSent=0;
          FILE *fp = fopen(fileToTransfer,"rb");
          if(fp==NULL){cout<<"FileTransferFail\n"; perror("File not present or not readable"); close(client_sockets[i]);client_sockets[i] = 0;break;}
          while(true){
            unsigned char buff[1024]={0};
            int bytes_read = fread(buff,1,1024,fp);
            if(bytes_read > 0){
              totalbytesSent+=bytes_read;
              if(send(client_sockets[i], buff, bytes_read,0)<0){
              	 close(client_sockets[i]);client_sockets[i] = 0;break;
              }
            }
            if (bytes_read < 1024){
              if (ferror(fp)) cout<<"Error reading file\n";
              break;
            }
          }
          
          cout<<"Transfer Done: "<<totalbytesSent<<" bytes\n";
          close(client_sockets[i]);
          client_sockets[i] = 0;
        }
      }  
    }


  }

}