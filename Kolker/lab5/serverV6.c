#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

time_t get_time_char(char* buf) // получение времени в массив и time_t 
{
    time_t It,Copy;
    It = time(NULL);
    Copy = It;
    printf("%ld - time on server\n",It);
    for (int i = 0;i<sizeof(time_t)+2;i++)
    {
        buf[sizeof(time_t) + 1 - i] = It % 10;
        It = It / 10;
    }
    return Copy;
}

int comparison(time_t serv,time_t client,char* buf) //сравнение
{
    memset(buf,0,sizeof(time_t)+2);
    int com = (int)(client - serv);
    int resv = com;
    //printf("%d --- in comparison\n",com);
    int i = 0;
    for (i;resv > 0;i++) resv = resv / 10;
    int copy_i = i;
    while(i > -1)
    {  
        buf[i-1] = com % 10;
        com = com / 10;
        i--;
    }
    for (int i = 0;i<copy_i;i++) printf("%d",buf[i]);
    printf(" --- in comparison\n");
    return(copy_i);
}


double power(double x, long n) //возведение в степень
{ 
    if(n == 0) return 1;
    if(n < 0) return power ( 1.0 / x, -n);
    return x * power(x, n - 1);
}


time_t get_char_in_time_t(char* buf) // преобразование из массива в time_t
{
    time_t time = 0;
    for (int i = 0;i<sizeof(time_t)+2;i++)
    {
        time = time + buf[sizeof(time_t) + 1 - i] * power(10,i);
    }
    return time;
}


int main() {
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in6 server_address;
    struct sockaddr_storage client_address;

    char *buf;
    buf = (char*) calloc (sizeof(time_t)+2, sizeof(char));

 
    server_sockfd = socket(AF_INET6, SOCK_STREAM, 0);

    server_address.sin6_family = AF_INET6;   ///интернет домен
    server_address.sin6_port = htons(56000);    /// 21 порт
    inet_pton(AF_INET6,"::1" , &server_address.sin6_addr);
    server_len = sizeof(server_address);
    if (bind(server_sockfd, (struct sockaddr *)&server_address, server_len) < 0)
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    listen(server_sockfd, 5);

    while(1) {

        char *ch;
        ch = (char*) calloc (sizeof(time_t)+2, sizeof(char));

        time_t It = get_time_char(buf);
        time_t It_from_c = 0;
        printf("server waiting\n");

        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd,
        (struct sockaddr *)&client_address, &client_len);


        read(client_sockfd, ch, 10);
        It_from_c = get_char_in_time_t(ch);

        int len_ch = comparison(It,It_from_c,ch);


        write(client_sockfd, ch, len_ch);
        close(client_sockfd);
    }
}
