#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h> 
#include <netinet/in.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <zmq.h>
#include <zlib.h>



void init_ZMQ(void);
void Send_ZMQ(char *,int *,int,unsigned char);
void init_UDP(void);
void Send_UDP(char *,int *,int,unsigned char);

unsigned int Frame_ctr=0;  //Главный счетчик кадров 8и битного потока
unsigned int Frame_ctr2=0;  //Главный счетчик кадров 16и битного потока

void *publisher; // ZMQ
void *context;

int udp_socket; // Сокет UDP
struct sockaddr_in addr_UDP_BR; // Адрес для бродкаста в рамках текущего сеанса

/* размер фрейма кодирования - 160 отсчетов  */

#define FRAME_SIZE 160 
#define FRAME_SIZE_MAX 1500 

int main(int argc, char **argv)
{
   char *inFile;  
   char *inFile2;

   FILE *fin;// файлы для ввода данных
   FILE *fin2;
   char in[FRAME_SIZE*2]; // Т.к. у нас массив данных 8 и битное знаковое число
   char Zin[FRAME_SIZE_MAX]; // Т.к. у нас массив данных 8 и битное знаковое число

   int i, tmp,rc;

   inFile = argv[1];
   inFile2 = argv[2];   
   fin = fopen(inFile, "rb");
   fin2 = fopen(inFile2, "rb");

   if(fin==NULL || fin2==NULL) perror("input voice file error");
   /* Инициализируем систему ZMQ */
   init_ZMQ();   
   /* Инициализируем систему UDP */
   init_UDP();   
   fseek(fin,48L,SEEK_SET);// пропустили заголовок RIFF, который создает Arecord в потоке 8 бит
   fseek(fin2,48L,SEEK_SET);// пропустили заголовок RIFF, который создает Arecord в потоке 16 бит

   while(1) // общий цикл для вещания всех потоков
       {   
       while (1)
           {
           /*Читаем аудиофайл ( предполагается стандартный формат 8 бит на отсчет 8000 Гц 1 канал)*/
           if( fread(in, 1, FRAME_SIZE, fin)==0) perror("read voice file");; // забрали 160 отсчетов
           if( feof(fin) ) // Достигли конца файла= пауза 5 секунд тишины
               {
                 printf("FULL LOOP1 %d\n\r", Frame_ctr);         
                 fseek(fin,48L,SEEK_SET); // установили указатель чтения на начало данных 
                 break; // перешли к вещанию другого потока
               }
         
          Send_UDP((char *)in,&Frame_ctr,FRAME_SIZE,0);// Отправили фрейм тип RA8 в UDP
          Send_ZMQ((char *)in,&Frame_ctr,FRAME_SIZE,0);// Отправили фрейм тип RA8 в ZMQ
          unsigned long Zlen=FRAME_SIZE*2; // максимальный розмер буфера,чтобы не случилось переполнение
          rc=compress((Bytef *)Zin,&Zlen,( Bytef *)in,FRAME_SIZE);
          if(rc!=Z_OK) perror("zlib");  
          Send_UDP((char *)Zin,&Frame_ctr,Zlen,1);// Отправили фрейм тип ZA8 в UDP
          Send_ZMQ((char *)Zin,&Frame_ctr,Zlen,1);// Отправили фрейм тип Z08 в ZMQ
          Frame_ctr++;
          usleep(18000); // пауза
          }
   
       while (1)
           {

           //Читаем аудиофайл ( предполагается стандартный формат 8 бит на отсчет 8000 Гц 1 канал)
           if( fread(in, 2, FRAME_SIZE, fin2)==0) perror("read voice file 2");; // забрали 160 отсчетов
           if( feof(fin2) ) // Достигли конца файла= пауза 5 секунд тишины
               {
                 printf("FULL LOOP %d\n\r", Frame_ctr2);        
                 fseek(fin2,48L,SEEK_SET); // установили указатель чтения на начало данных 
                 break; // перешли к вещанию другого потока
               }
         
          Send_UDP((char *)in,&Frame_ctr2,FRAME_SIZE*2,2);// Отправили фрейм тип R16 в UDP
          Send_ZMQ((char *)in,&Frame_ctr2,FRAME_SIZE*2,2);// Отправили фрейм тип R16 в ZMQ
          unsigned long Zlen=FRAME_SIZE_MAX; // максимальный розмер буфера,чтобы не случилось переполнение
          rc=compress((Bytef *)Zin,&Zlen,(Bytef *)in,FRAME_SIZE*2);
          if(rc!=Z_OK) perror("zlib");  
          Send_UDP((char *)Zin,&Frame_ctr2,Zlen,3);// Отправили фрейм тип Z16 в UDP
          Send_ZMQ((char *)Zin,&Frame_ctr2,Zlen,3);// Отправили фрейм тип Z16 в ZMQ
          Frame_ctr2++;
          usleep(10000); // пауза
          }
    sleep(10);//10 секунд между потоками

    }
   fclose(fin);fclose(fin2);
   return 0;
}

void init_UDP(void) // Инициализируем систему рассылки UDP
{
    int rc;
    int aparam=1;
    struct sockaddr_in addr; // структура с типом адресов
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0); // инициализировали сокет
    if(udp_socket < 0)
    {
        perror("socket");
        exit(1);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345); // Будем вещать с порта 12345 для опрелеленности
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // привязали сокет
    if(bind(udp_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }
    rc=setsockopt(udp_socket,SOL_SOCKET,SO_BROADCAST,&aparam,sizeof(aparam)); // Включили широковещательный режим
    if(rc==-1)
       {
       perror("UDP_PARAM BRODCAST");exit(0);
       }


    addr_UDP_BR.sin_family = AF_INET;
    addr_UDP_BR.sin_port = htons(3301);
    addr_UDP_BR.sin_addr.s_addr = htonl(INADDR_ANY);
}

void init_ZMQ(void) // Инициализируем систему рассылки ZMQ
{

    context=zmq_init (2); if(context==NULL) exit(0); // создали контекст
    publisher = zmq_socket (context, ZMQ_PUB);assert (context); // создали 0mq гнездо- publisher     
    zmq_bind(publisher, "tcp://*:5572");

}

void Send_ZMQ(char *message,int *ctr,int len,unsigned char flag) // Отправка пакета через ZMQ.
{
    zmq_msg_t msg;
    int rc;
    
    unsigned char buf[1500-46-20-4]; // буфер не больше MTU-заголовок ETH- заголовок IP - признак канала= 1434 
    if(len>1434) return;// ошибочный пакет
// Формируем структуру пакета
    if(flag==0) memcpy(buf,"R08",3);// скопировали 3 байт признака канала 
    if(flag==1) memcpy(buf,"Z08",3);// скопировали 3 байт признака канала 
    if(flag==2) memcpy(buf,"R16",3);// скопировали 3 байт признака канала 
    if(flag==3) memcpy(buf,"Z16",3);// скопировали 3 байт признака канала 

    memcpy(buf+3,ctr,4);// Заполнили метку времени
    memcpy(buf+7,message,len);// Заполнили содерджимое пакета;
    
    rc = zmq_msg_init_size (&msg, len+7); assert (rc == 0); // подготовили объект
    rc = zmq_msg_init_data (&msg, buf,len+7, NULL, NULL); assert (rc == 0); // заполнили данными
    zmq_sendmsg(publisher,&msg,0); 
//было в версии 2.2    rc = zmq_send (publisher, &msg, 0); assert (rc == 0);// опубликовали структуру


}


void Send_UDP(char *message,int *ctr,int len,unsigned char flag) // Отправка пакета через ZMQ.
{
    char buf[1500-46-20-4]; // буфер не больше MTU-заголовок ETH- заголовок IP - признак канала= 1434 
    if(len>1434) return;// ошибочный пакет
// Формируем структуру пакета
    if(flag==0) memcpy(buf,"R08",3);// скопировали 3 байт признака канала 
    if(flag==1) memcpy(buf,"Z08",3);// скопировали 3 байт признака канала 
    if(flag==2) memcpy(buf,"R16",3);// скопировали 3 байт признака канала 
    if(flag==3) memcpy(buf,"Z16",3);// скопировали 3 байт признака канала 

    memcpy(buf+3,ctr,4);// Заполнили метку времени
    memcpy(buf+7,message,len);// Заполнили содерджимое пакета;

	//отправили фрейм
    if(sendto(udp_socket,(char *)buf,len+7,0,(const struct sockaddr *)&addr_UDP_BR,sizeof(addr_UDP_BR) ) ==-1)
        {
        perror("UDP_SEND");
        }
}