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
  fprintf(stderr, "%s <serverIPAddr:port> <fileToReceive>\n",
	  progname);
} // End usage()


int main(int argc, char *argv[]){
	/* Get command-line arguments */
	if(argc != 3) { usage(argv[0]); exit(1); }
	string ip_port = argv[1];
	int i=ip_port.find(":");
	const char* ip = ip_port.substr(0,i).c_str();
	const char* p = ip_port.substr(i+1,ip_port.length()-i-1).c_str();
	int port = atoi(p);
	string fileToReceive=argv[2];


	int socket_fd;
	socket_fd = socket(PF_INET,SOCK_STREAM,0);
	if(socket_fd < 0) { perror("socket failed"); exit(20); }

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr.s_addr = inet_addr(ip);


  int c = connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if(c==-1) {perror("Connection to server failed"); exit(2);}

  cout<<"connectDone: "<<inet_ntoa(serv_addr.sin_addr)<<":"<<ntohs(serv_addr.sin_port)<<endl;

  int sent=0;

  string get_filename = "get "+fileToReceive;
  cout<<get_filename<<endl;
  const char* send_filename = get_filename.c_str();
  send(socket_fd,send_filename,get_filename.length(),0);

  int bytesReceived = 0,totalbytesReceived=0;
  char recvBuff[1024];
  memset(recvBuff, '0', sizeof(recvBuff));
  FILE *fp;

  //fp = fopen(fileToReceive.c_str(), "wb");
  fp = fopen("client2.txt","wb"); 
  if(NULL == fp){
     perror("Unable to create/write file");
     exit(3);
  }
  /* Receive data in chunks of 256 bytes */
  while(true){ 
    bytesReceived = recv(socket_fd, recvBuff, 1024,0);
    if(bytesReceived>0){
      totalbytesReceived+=bytesReceived;
      fflush(stdout);
      fwrite(recvBuff, 1,bytesReceived,fp);
    }
    else break;
  }

  cout<<"File Written : "<<totalbytesReceived<<" bytes\n";

  }