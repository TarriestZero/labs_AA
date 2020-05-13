#include <stdlib.h> 
#include <string.h>
#include <sys/ioctl.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <netinet/in.h>
#include <sys/soundcard.h> 
#include<arpa/inet.h> 
#include <stdio.h> 
#include <sys/types.h>
#include <sys/socket.h>

#define AUDIO_DEVICE "/dev/dsp"
#define pakage 6

void init_UDP(void);

int udp_socket; // Сокет UDP
struct sockaddr_in adr; // Адрес для бродкаста в рамках текущего сеанса


int main ()
{
    printf("Start client.... \n");
    short int Size_for = 1 + (167 * pakage);
    unsigned char buf[1200];
    char * buffer = (char*) malloc(sizeof(int) * 500); // выделить память для хранения содержимого файла
    init_UDP();
    int len = sizeof(buf);
    int format,nchans,rate;
    int bytes_read;
    int res,res2,audio_fd;
    printf("SETTINGS\n\r"); 
    audio_fd = open(AUDIO_DEVICE, O_WRONLY);
    if(audio_fd<0) { printf("no dsp\n\r") ;exit(0); }
    format = AFMT_U8;
    ioctl(audio_fd, SNDCTL_DSP_SETFMT, &format);
    nchans = 1;
    ioctl(audio_fd, SNDCTL_DSP_CHANNELS, &nchans);
    rate = 8000;
    ioctl(audio_fd, SNDCTL_DSP_SPEED, &rate);
    ioctl(audio_fd, SOUND_PCM_READ_RATE, &rate);
    printf("Recording mode: 1000 samples\n\r");

    for(;;)
    {
        memset(buffer, 0, sizeof(buffer)); 
        int n = 0;
        for(int co = 0; co < Size_for;){
        bytes_read = recvfrom(udp_socket, buffer, sizeof(buf),  
                            0, (struct sockaddr *) &adr, 
                            &len); 
        if ((buffer[0] == 82) && (buffer[1] == 48)){
        co = co + bytes_read;
        for (int co1 = 3;(co - (3 * (co / 167)) > n); n++, co1++){
        buf[n]= buffer[co1];  // в buf складируется примерно 6 частей 
        }
        //n++;
        }
        }

        if ((buffer[0] == 82) && (buffer[1] == 48)){  //роверка на тип
            res2=write(audio_fd,&buffer[7],bytes_read - 7); //пушаем в звуковуху с 7 элемента (сами данные)             
        }
    }
    close(udp_socket);
    return(0);
}



void init_UDP(void) // Инициализируем систему рассылки UDP
{
    memset(&adr, 0, sizeof(adr));
    int rc;
    int aparam=1;
    //struct sockaddr_in ad; // структура с типом адресов
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0); // инициализировали сокет
    if(udp_socket < 0)
    {
        perror("socket");
        exit(1);
    }
    adr.sin_family = AF_INET;
    adr.sin_port = htons(3301); // принимаем на этот порт
    inet_aton("127.0.0.1" , &adr.sin_addr);
    // привязали сокет
    if(bind(udp_socket, (struct sockaddr *)&adr, sizeof(adr)) < 0)
    {
        perror("bind");
        exit(2);
    }
    rc=setsockopt(udp_socket,SOL_SOCKET,SO_BROADCAST,&aparam,sizeof(aparam)); // Включили широковещательный режим
    if(rc==-1)
       {
       perror("UDP_PARAM BRODCAST");exit(0);
       }
}
