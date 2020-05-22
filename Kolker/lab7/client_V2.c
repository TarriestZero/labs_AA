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
#define frame 167

#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define SUCCESS        0


void init_UDP(void);

int udp_socket; // Сокет UDP
struct sockaddr_in adr; // Адрес для бродкаста в рамках текущего сеанса



int set_audio_dev(){

    int audio_fd;
    int format,nchans,rate;
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
    return audio_fd;
}


        struct p
        {
            unsigned char *buf;
            int time;
        };

int main ()
{
    init_UDP();
    int audio_fd = set_audio_dev();
    for(;;)
    {
        unsigned char buffer[frame + 3] = {0};
        int len_bufrecv = sizeof(buffer);
        unsigned char temp_buf[frame] = {0};
        struct p pack[pakage];

        for(int i = 0; i < pakage; i++) 
        {
            pack[i].buf = calloc(frame,sizeof(unsigned char));
        }
        for(int co = 0; co < pakage;)
        {
            recvfrom(udp_socket, buffer, frame,  
                                0, (struct sockaddr *) &adr, 
                                &len_bufrecv); 
            if ((buffer[0] == 82) && (buffer[1] == 48))
            {
                pack[co].time = *((int*)&buffer[3]);
                for (int co1 = 7;co1 < frame; co1++)
                    pack[co].buf[co1 - 7] = buffer[co1];
                co++;
            }
        }
        for (int i = 0; i < pakage - 1; i++) //пузырьком сравниваем метки времени
        {
            for(int j = pakage - 1; j > i; j--)
            {
                if(pack[j - 1].time > pack[j].time)
                {
                    unsigned char *tmpbuf = pack[j - 1].buf;
                    int tmptime = pack[j - 1].time;
                    pack[j - 1] = pack[j];
                    pack[j].buf = tmpbuf;
                    pack[j].time = tmptime;
                }    
            }
        }
        
        for(int i = 0; i < pakage; i++)
            write(audio_fd,pack[i].buf,frame - 7);
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
