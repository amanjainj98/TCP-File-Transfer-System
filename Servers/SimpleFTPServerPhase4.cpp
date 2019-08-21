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
#include<map>
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
    fd_set writefds;
    vector<int> client_sockets;
    map<int,FILE*> pending_files;
    map<int,FILE*> pending_read_files;
    map<int,int> total_bytes_sent;
    map<int,int> total_bytes_received;

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
    int max_socket_fd = socket_fd;
    for(int i = 0;i < client_sockets.size();i++){  
      if(client_sockets[i] > 0){
        FD_SET(client_sockets[i], &readfds);
        FD_SET(client_sockets[i], &writefds);

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
            if(recvBuff[0]=='g'&&recvBuff[1]=='e'&&recvBuff[2]=='t'){
                char* fileToTransfer = recvBuff+4;
                cout<<"FileRequested: "<<fileToTransfer<<endl;
                FILE *fp = fopen(fileToTransfer,"rb");
                pending_files[i] = fp;
                total_bytes_sent[i] = 0;
            }

            else if(recvBuff[0]=='p'&&recvBuff[1]=='u'&&recvBuff[2]=='t'){
                char* fileToReceive = recvBuff+4;

                cout<<"FileToReceive: "<<fileToReceive<<endl;
                //cout<<"fopen\n";
                FILE *fp = fopen(fileToReceive,"wb");
                pending_read_files[i] = fp;
                total_bytes_received[i] = 0;
            }

            else{
                //cout<<"aa\n";
                if(bytesReceived>0){
                  total_bytes_received[i]+=bytesReceived;
                  fflush(stdout);
                  fwrite(recvBuff, 1,bytesReceived,pending_read_files[i]);
                }
                if(bytesReceived<1024){
                    cout<<"Receive Done: "<<total_bytes_received[i]<<" bytes\n";
                    close(client_sockets[i]);
                    client_sockets[i] = 0;
                    fclose(pending_read_files[i]);
                    pending_read_files.erase(i);
                    total_bytes_received.erase(i);
                }
            }
        }
      }  
    }

    for (map<int,FILE*>::iterator it=pending_files.begin(); it!=pending_files.end(); ++it){
      int i = it->first;
      if(it->second==NULL){cout<<"FileTransferFail\n"; perror("File not present or not readable"); close(client_sockets[i]);client_sockets[i] = 0;break;}
      unsigned char buff[1024]={0};
      int bytes_read = fread(buff,1,1024,it->second);
      if(bytes_read > 0){
        total_bytes_sent[i]+=bytes_read;
        if(send(client_sockets[i], buff, bytes_read,0)<0){
            close(client_sockets[i]);client_sockets[i] = 0;break;
        }
      }
      if (bytes_read < 1024){
        if (ferror(it->second)) cout<<"Error reading file\n";
          cout<<"Transfer Done: "<<total_bytes_sent[i]<<" bytes\n";
          close(client_sockets[i]);
          client_sockets[i] = 0;
          pending_files.erase(i);
          total_bytes_sent.erase(i);
        break;
      }

    }


  }

}