#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h> 
#include<netinet/in.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>


#define PORT     8080 
#define MAXLINE 1024 

int udp_socket = -1;
char buf[MAXLINE];
char bufup[MAXLINE];
void init_UDP(void);
unsigned char xorbyte=0;

int main()
{
    memset(&buf, 0, sizeof(buf)); 
    struct sockaddr_in6     servaddr; 
    socklen_t  structlen = sizeof(servaddr);
    // Creating socket file descriptor 
    if ( (udp_socket = socket(AF_INET6, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(5000);
    inet_pton(AF_INET6, "::1", &servaddr.sin6_addr);

    buf[0]=129;// ключик
    buf[1]=30; // день
    buf[2]=03; // месяц
    buf[3]=8; // номер бригады
    sprintf(&buf[4],"Vladislav Mamatov");
    
    int len = sizeof(buf);

    for(int i=0;i<len;i++) xorbyte=xorbyte^buf[i];
    buf[81]=xorbyte;
    sendto(udp_socket, (const char *)buf, strlen(buf), 
           0, (const struct sockaddr *) &servaddr,  
           sizeof(servaddr)); 
    int bytes_read;

    bytes_read = recvfrom(udp_socket, (char *)bufup, MAXLINE,  
                          0, (struct sockaddr *) &servaddr, 
                          &structlen); 
    printf("recv %d bytes: \n",bytes_read);
    printf("%s\n", &bufup[0]);
    printf("%d\n", 256 + bufup[18]);
    printf("%d\n", bufup[19]);
    close(udp_socket);
    return 0;

}

