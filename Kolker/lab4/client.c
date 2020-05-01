#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h> 
#include<netinet/in.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>

#define PORT     8080 
#define MAXLINE 1024 

int udp_socket = -1;
char buf[MAXLINE];
char bufup[MAXLINE];
unsigned char xorbyte=0;

int main()
{

    struct sockaddr_in     servaddr; 
  
    // Creating socket file descriptor 
    if ( (udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = "127.0.0.1"; 

    buf[0]=129;// ключик
    buf[1]=04; // день
    buf[2]=04; // месяц
    buf[3]=8; // номер бригады
    sprintf(&buf[4],"Vladislav Mamatov");
    
    int len = sizeof(buf);

    for(int i=0;i<len;i++) xorbyte=xorbyte^buf[i];
    printf("xorbytes - %d\n",xorbyte);
    buf[81]=xorbyte;
    sendto(udp_socket, (const char *)buf, strlen(buf), 
           0, (const struct sockaddr *) &servaddr,  
           sizeof(servaddr)); 


    int bytes_read = recvfrom(udp_socket, (char *)bufup, MAXLINE,  
                          0, (struct sockaddr *) &servaddr, 
                          &len); 
    printf("recv %d bytes: \n",bytes_read);
    printf("%s\n", &bufup[0]);
    printf("%d\n", bufup[18]);
    printf("%d\n", bufup[19]);
    close(udp_socket);
    return 0;

}
