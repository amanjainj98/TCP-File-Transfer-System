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
  fprintf(stderr, "%s <serverIPAddr:port> <op> <fileToReceive> <receiveInterval>\n",
    progname);
} // End usage()


int main(int argc, char *argv[]){
  /* Get command-line arguments */
  if(argc != 5) { usage(argv[0]); exit(1); }
  string ip_port = argv[1];
  int i=ip_port.find(":");
  const char* ip = ip_port.substr(0,i).c_str();
  const char* p = ip_port.substr(i+1,ip_port.length()-i-1).c_str();
  int port = atoi(p);
  char* op = argv[2];
  string fileToReceive=argv[3];
  double receiveInterval = atof(argv[4]);


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
  
  if(op[0]=='g'&&op[1]=='e'&&op[2]=='t'){
    string get_filename = "get "+fileToReceive;
    //cout<<get_filename<<endl;
    char* send_filename = new char[get_filename.length()+1];
    strcpy(send_filename,get_filename.c_str());
    send_filename[get_filename.length()]='\0';
    send(socket_fd,send_filename,get_filename.length()+1,0);

    int bytesReceived = 0,totalbytesReceived=0,temp_bytes=0;
    char recvBuff[1024];
    memset(recvBuff, '0', sizeof(recvBuff));
    FILE *fp;

    //fp = fopen(fileToReceive.c_str(), "wb");
    fp = fopen(fileToReceive.c_str(),"wb"); 
    if(NULL == fp){
      perror("Unable to create/write file");
      exit(3);
    }

    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int base_time = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    struct timeval tp1;

    /* Receive data in chunks of 256 bytes */
    while(true){ 
      gettimeofday(&tp1, NULL);
      long int curr_time = tp1.tv_sec * 1000 + tp1.tv_usec / 1000;

      if((curr_time-base_time<receiveInterval)&&temp_bytes<=950){
        bytesReceived = recv(socket_fd, recvBuff, 50,0);
        if(bytesReceived>0){
          totalbytesReceived+=bytesReceived;
          temp_bytes+=bytesReceived;
          fflush(stdout);
          fwrite(recvBuff, 1,bytesReceived,fp);
        }
        else break;
      }

      else if(curr_time-base_time<receiveInterval){
        //cout<<"waiting....";
        while(true){
          gettimeofday(&tp1, NULL);
          long int curr_time = tp1.tv_sec * 1000 + tp1.tv_usec / 1000;
          if(curr_time-base_time>=receiveInterval){ break; }
        }
        base_time+=receiveInterval;  
        temp_bytes=0;     
      }

      else{
        base_time+=receiveInterval;  
        temp_bytes=0;       
      }
    }


    cout<<"File Written : "<<totalbytesReceived<<" bytes\n";
  }

	else if(op[0]=='p'&&op[1]=='u'&&op[2]=='t'){
    string put_filename = "put "+fileToReceive;
    //cout<<get_filename<<endl;
    char* send_filename = new char[put_filename.length()+1];
    strcpy(send_filename,put_filename.c_str());
    send_filename[put_filename.length()]='\0';
    //send(socket_fd,send_filename,put_filename.length()+1,0);
    sleep(1);
    char* fileToTransfer = new char[fileToReceive.length() + 1];
    strcpy(fileToTransfer, fileToReceive.c_str());
    if(access(fileToTransfer,R_OK) == -1 ) {cout<<"FileTransferFail\n"; perror("File not present or not readable"); exit(3); }
    int totalbytesSent=0;
    FILE *fp = fopen(fileToTransfer,"rb");
    if(fp==NULL){cout<<"FileTransferFail\n"; perror("File not present or not readable"); exit(3); }
    while(true){
      unsigned char buff[1024]={0};
      int bytes_read = fread(buff,1,1024,fp);
      if(bytes_read > 0){
        totalbytesSent+=bytes_read;
        send(socket_fd, buff, bytes_read,0);
      }
      if (bytes_read < 1024){
        if (ferror(fp)) cout<<"Error reading file\n";
        break;
      }
    }
    
    cout<<"Transfer Done: "<<totalbytesSent<<" bytes\n";

	}

  }