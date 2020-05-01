#include<stdlib.h>
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<fcntl.h> // for open
#include<unistd.h> // for close
#include<netdb.h>
#include<time.h>


#define SERVERIP "::1"

int main() {
    int sockfd, port;
    int sizeimage = 10000000; 
    char *resultimage;
    resultimage = (char*) calloc (sizeimage, sizeof(char));

 
    struct sockaddr_in6 server; 
    port = 80;
    //Create socket
    sockfd = socket(AF_INET6 , SOCK_STREAM , 0);//IPv4, STREAM NOT DDGRAM, TCP
 
    if (sockfd < 0) {
        printf("Could not create socket\n");
    }
 
    printf("Socket created\n");
 
    server.sin6_family = AF_INET6; 
    server.sin6_port = htons(port); 
    inet_pton(AF_INET6,SERVERIP , &server.sin6_addr);
 
    //Connect to remote server
    if (connect(sockfd , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
 
    char request[256];//Send this message to server
    char server_reply[1000000];//buffer for server reply
 

    printf("Choos, what you want:\n 1-http_gyper\n2-image\n");
    int s = getchar();
    switch (s)
    {
    case 49:
    sprintf(request, "%s", "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n\r\n");
        break;
    case 50:
    sprintf(request, "%s", "GET /image/image.png HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n\r\n");
        break;            
    default:
    printf("Wrong");
        exit;
    }

    //Send some data with TCP
    if(send(sockfd, request, strlen(request), 0) < 0) {
        printf("Send failed\n");
        exit(1);
    }
 
 
    printf("Request sended\n");
    
    int cnt = 1;
    int cc = 0;
    char *copy = resultimage;
    
    while(cnt > 0)  // рием
   {
        char buf8[8];
        cnt = recv(sockfd, buf8, 8, 0);
        for (int i=0; i < cnt; i++) resultimage[i+cc] = buf8[i];
        cc += cnt;

   }

    resultimage = strstr(resultimage, "\r\n\r\n");//save data after header
    resultimage+=4;//Тут читать только данные без http header
    //printf("%d",cc);
    if (s = 50) //  Запись в файл картинки
    {
    FILE *fp;
    if((fp=fopen("file", "w"))==NULL) 
    {
    printf("Cannot open file.");
    return 1;
    }
    fwrite(resultimage, sizeof(char)*cc, 1, fp);
    fclose (fp);
    }
    for (int h;500 > h;h++) printf("%c", resultimage[h]);//print the answer
    close(sockfd);
    
    return 0;
}