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


#define PORT     8080 
#define MAXLINE 1024 


 int udp_socket=-1; // идентификатор сокет для UDP
 char udpbuf[1024]; // буфер для хранения данных
 
struct sockaddr_in addr_UDP; //Структура для хранения адреса

void init_UDP(void);// инициализация UDP сокет
void send_UDP(int); // Отправляем данные в UDP


void main()
{
int i;
int recv;
int structlen;
char SENDER_ADDR[44];
int byte1,byte2,byte3,byte4;
unsigned char xorbyte=0;

struct sockaddr_in src_addr_UDP; //Структура для хранения отправителя

init_UDP(); // Инициализировали UDP сокет

while(1) // Вечный цикл
    {

    memset(&src_addr_UDP, 0, sizeof(SENDER_ADDR)); 
    structlen = sizeof(SENDER_ADDR);
    recv=recvfrom(udp_socket,(char *)udpbuf, 1024, MSG_WAITALL,(struct sockaddr *)&src_addr_UDP,&structlen);
    printf("recv %d bytes: \n",recv); // напечатали сколько байт пришло
    
    if(recv>0 && (unsigned char) udpbuf[0]==129) // если ключик верный смотрим кто послал
     {
    byte1=((int)src_addr_UDP.sin_addr.s_addr&0xff000000)>>24;
    byte2=((int)src_addr_UDP.sin_addr.s_addr&0xff0000)>>16;
    byte3=((int)src_addr_UDP.sin_addr.s_addr&0xff00)>>8;
    byte4=((int)src_addr_UDP.sin_addr.s_addr&0xff);   
    sprintf(SENDER_ADDR,"%d.%d.%d.%d",byte4,byte3,byte2,byte1);

    if( strlen(&udpbuf[4])<20 )
	{      
        printf("IP#%s BR#%d N1:%s\n",SENDER_ADDR,udpbuf[3],&udpbuf[4] ); // все верно
    }
    else
	{
	    printf("IP#%s BR#%d N1:badname:%s\n",SENDER_ADDR,udpbuf[3],&udpbuf[4] );// есть проблемы с посылкой
	}


    
	for(i=0;i<recv;i++) 
    {
    xorbyte=xorbyte^udpbuf[i]; //сделали исключаюшее или всех байтиков
    }
	printf("%d -----xorbyte before send\n",xorbyte);
    sprintf(udpbuf,"END_OF_WINTER!2018");// вот тут секретное словечко,которое будем отвечать в канал
	udpbuf[18]=(char)xorbyte;// контрольный XOR
	udpbuf[19]=0x0;//А вот тут будет секретный код,который мы будем посылать
	//отправили назад кодовое слово тому,кто нам прислал привет
    if(sendto(udp_socket,(char *)udpbuf,sizeof(udpbuf),MSG_CONFIRM,(const struct sockaddr *)&src_addr_UDP,sizeof(src_addr_UDP) ) ==-1)
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

    struct sockaddr_in servaddr; // структура с типом адресов


     if ( (udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) //
    { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    memset(&servaddr, 0, sizeof(servaddr));   
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
    // привязали сокет
    if ( bind(udp_socket, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

}

