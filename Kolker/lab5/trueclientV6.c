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

void get_time_char(char* buf) //достаем время машины в массив char и time_t
{
    time_t It;
    It = time(NULL);
    printf("%ld-sss\n",It);
    for (int i = 0;i<sizeof(time_t)+2;i++)
    {
        buf[sizeof(time_t) + 1 - i] = It % 10;
        It = It / 10;
    }
}

double power(double x, long n) //возведение в степень
{ 
    if(n == 0) return 1;
    if(n < 0) return power ( 1.0 / x, -n);
    return x * power(x, n - 1);
}


void get_char_in_time_t(char* buf) // преобразование из массива в time_t
{
    time_t time = 0;
    for (int i = 0;i<sizeof(time_t)+2;i++)
    {
        time = time + buf[sizeof(time_t) + 1 - i] * power(10,i);
    }
    printf("%ld\n",time);
}

int main() {
 int sockfd;
 int len;
 struct sockaddr_in6 address;
 int result;
 
    char *buf;
    buf = (char*) calloc (sizeof(time_t)+2, sizeof(char));
    get_time_char(buf);
    sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    address.sin6_family = AF_INET6;   ///интернет домен
    address.sin6_port = htons(56000);    /// 21 порт
    inet_pton(AF_INET6,"::1" , &address.sin6_addr);
    int server_len = sizeof(address);

 result = connect(sockfd, (struct sockaddr *)&address, server_len);
 if (result == -1) 
 {
  perror("oops : client1");
  exit(1);
  }

 write(sockfd, buf, 10);


 memset(buf,0,sizeof(time_t)+2);
 read(sockfd, buf, 10);
 for (int i = 0;buf[i] != 0;i++) printf("%d",buf[i]);
 printf(" ---- разница в unix времени\n");
 close(sockfd);
 exit(0);
}
