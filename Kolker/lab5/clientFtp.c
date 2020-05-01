#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
 
int s;
int ds;
 
int init_sock() {
    int len;
    struct sockaddr_in address;
    int result;
    //int s;
    s = socket(AF_INET, SOCK_STREAM,0);
    address.sin_family = AF_INET;   ///интернет домен
    address.sin_addr.s_addr = inet_addr("192.168.88.249");   ///соединяемся с 127,0,0,1
    address.sin_port = htons(21);    /// 21 порт
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
    printf("Введите имя"); char name[64]; fgets(name,sizeof(name),stdin);
    char str[512];
    sprintf(str,"USER %s\r\n",name);
    send(s,str,strlen(str),0);
    readServ(s);
    printf("пароль"); char pass[64]; fgets(pass,sizeof(pass),stdin);
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

 
   close(s);  ///закрытие соединения
   return 0;
}