#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
int s;
int ds;

#define SERVERIP "fe80::ed22:e1db:ec26:48b2"

int init_sock() {
    int len;
    struct sockaddr_in6 address;
    int result;
    //int s;
    s = socket(AF_INET6, SOCK_STREAM,0);
    address.sin6_family = AF_INET6;   ///интернет домен
    address.sin6_port = htons(21);    /// 21 порт
    address.sin6_scope_id = if_nametoindex("ens33"); // указываем интерфейс
    inet_pton(AF_INET6, SERVERIP , &address.sin6_addr);
    len = sizeof(address);
    result = connect(s, (struct sockaddr *)&address, len);   ///установка соединения
    if (result == -1) {
        perror("oops: client");
        return -1;
    }
    return s;
}
 
 
 
int readServ(int s) {
    int rc;
    fd_set fdr;
    FD_ZERO(&fdr);
    FD_SET(s,&fdr);
    struct timeval timeout;
    timeout.tv_sec = 1;   ///зададим  структуру времени со значением 1 сек
    timeout.tv_usec = 0;  
    do {
        char buff[512] ={' '};
        recv(s,&buff,512,0);   ///получаем данные из потока
        printf("%s",buff);
        rc = select(s+1,&fdr,NULL,NULL,&timeout);    ///ждём данные для чтения в потоке 1 сек. 
    } while(rc);     ///проверяем результат
    return 2;
}
 

 
  
  int login() {
    printf("Введите имя\n"); char name[64]; fgets(name,sizeof(name),stdin);
    char str[512];
    sprintf(str,"USER %s\r\n",name);
    send(s,str,strlen(str),0);
    readServ(s);
    printf("пароль\n"); char pass[64]; fgets(pass,sizeof(pass),stdin);
    sprintf(str,"PASS %s\r\n",pass);
    send(s,str,strlen(str),0);
    readServ(s);
    return 0;
}
 
int main() {
   char sym[sizeof("PWD\r\n")] = "PWD\r\n";
   s = init_sock();
   readServ(s);
   
   login();
   send(s,sym,strlen(sym),0);
   readServ(s);
   send(s,"help\r\n",strlen("help\r\n"),0);
   readServ(s);
   send(s,"cwd fff\r\n",strlen("cwd fff\r\n"),0);
   readServ(s);
   send(s,"PWD\r\n",strlen("PWD\r\n"),0);
   readServ(s);

 
   close(s);  ///закрытие соединения
   return 0;
}