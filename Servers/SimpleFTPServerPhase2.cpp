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

	//cout<<portNum<<endl<<fileToTransfer<<endl;
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
	cout<<l<<endl;
  label:
	socklen_t sin_size = sizeof(struct sockaddr_in);
	int new_fd = accept(socket_fd, (struct sockaddr *)&their_addr, &sin_size);
	if(new_fd==-1) {perror("Accept failed"); exit(2);}
	cout<<"Client: "<<inet_ntoa(their_addr.sin_addr)<<":"<<ntohs(their_addr.sin_port)<<endl;


  int bytesReceived = 0;
  char recvBuff[1024] = {0};
  memset(recvBuff, '0', sizeof(recvBuff));

  bytesReceived = read(new_fd, recvBuff, 1024);
  recvBuff[bytesReceived]='\0';

  char* fileToTransfer = recvBuff+4;

  cout<<"FileRequested: "<<fileToTransfer<<endl;


  if(access(fileToTransfer,R_OK) == -1 ) {cout<<"FileTransferFail\n"; perror("File not present or not readable");close(new_fd); goto label; }
  int totalbytesSent=0;
	FILE *fp = fopen(fileToTransfer,"rb");
  if(fp==NULL){cout<<"FileTransferFail\n"; perror("File not present or not readable"); close(new_fd); goto label; }
  while(true){
    unsigned char buff[1024]={0};
    int bytes_read = fread(buff,1,1024,fp);
    if(bytes_read > 0){
      totalbytesSent+=bytes_read;
      if(send(new_fd, buff, bytes_read,0)<0){
      	close(new_fd);
        perror("Send Failed");goto label;
      }
    }
    if (bytes_read < 1024){
      if (ferror(fp)) cout<<"Error reading file\n";
      break;
	  }
  }
	
  cout<<"Transfer Done: "<<totalbytesSent<<" bytes\n";
  close(new_fd);

  goto label;
}