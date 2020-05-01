/*Программа сервера к лабораторной работе №4 */
/*2011 год*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h> 
#include<netinet/in.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>


#define PORT     8080 
#define MAXLINE 1024 

void *get_in_addr(struct sockaddr *sa) 
    { 
    if (sa->sa_family == AF_INET) 
    { 
        return &(((struct sockaddr_in*)sa)->sin_addr); 
    } 
    return &(((struct sockaddr_in6*)sa)->sin6_addr); 
    }


 int udp_socket=-1; // идентификатор сокет для UDP
 char udpbuf[1024]; // буфер для хранения данных
 
struct sockaddr_in6 addr_UDP; //Структура для хранения адреса

void init_UDP(void);// инициализация UDP сокет
void send_UDP(int); // Отправляем данные в UDP


void main()
{
int i;
int recv;
int structlen;
char SENDER_ADDR[INET6_ADDRSTRLEN];
int byte1,byte2,byte3,byte4;
unsigned char xorbyte=0;

struct sockaddr_storage src_addr_UDP; //Структура для хранения отправителя

init_UDP(); // Инициализировали UDP сокет


while(1) // Вечный цикл
    {
    memset(&udpbuf, 0, 1024); 
    memset(&src_addr_UDP, 0, sizeof(struct sockaddr_storage)); 
    socklen_t  structlen = sizeof(src_addr_UDP);
    recv=recvfrom(udp_socket,(char *)udpbuf, 1024, MSG_WAITALL,(struct sockaddr *)&src_addr_UDP,&structlen);
    printf("recv %d bytes: \n",recv); // напечатали сколько байт пришло
    
    if(recv>0 && (unsigned char) udpbuf[0]==129) // если ключик верный смотрим кто послал
    {
    inet_ntop(src_addr_UDP.ss_family,get_in_addr(
    (struct sockaddr *)&src_addr_UDP.__ss_padding),SENDER_ADDR, sizeof SENDER_ADDR); 

    if( strlen(&udpbuf[4])<20 )
	{      
        printf("IP#%s BR#%d N1:%s\n",SENDER_ADDR,udpbuf[3],&udpbuf[4] ); // все верно
    }
    else
	{
	    printf("IP#%d BR#%d N1:badname:%s\n",src_addr_UDP.ss_family,udpbuf[3],&udpbuf[4] );// есть проблемы с посылкой
	}

    for(i=0;i<recv;i++) 
    {
    xorbyte=xorbyte^udpbuf[i]; //сделали исключаюшее или всех байтиков
    }
    memset(&udpbuf, 0, 1024); 
	printf("%d ------ xor bytes before send\n",xorbyte);
    sprintf(udpbuf,"END_OF_WINTER!2018");// вот тут секретное словечко,которое будем отвечать в канал
	udpbuf[18]=(char)xorbyte;// контрольный XOR
    printf("%d",udpbuf[18]);
    printf("%s",&udpbuf[0]);
	udpbuf[19]=0x5;//А вот тут будет секретный код,который мы будем посылать
	//отправили назад кодовое слово тому,кто нам прислал привет
    if(sendto(udp_socket,(char *)udpbuf,sizeof(udpbuf),0,(const struct sockaddr *)&src_addr_UDP,sizeof(struct sockaddr_in6) ) ==-1)
        {
        perror("UDP_SEND");
        }
      }
      
// ждем 0.1 секунду
    usleep(100000); 
    }
}
void init_UDP(void) // инициализация сокет UDP
{

    struct sockaddr_in6 servaddr; // структура с типом адресов


     if ( (udp_socket = socket(AF_INET6, SOCK_DGRAM, 0)) < 0 ) 
    { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    memset(&servaddr, 0, sizeof(servaddr)); 
    
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(5000);
    servaddr.sin6_addr = in6addr_any;
    
    // привязали сокет
    if ( bind(udp_socket, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

}

