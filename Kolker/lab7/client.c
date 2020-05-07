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
#define MAXLINE 1024 

void sound_set_eth(void);
void init_UDP(void);

int udp_socket; // Сокет UDP
struct sockaddr_in adr; // Адрес для бродкаста в рамках текущего сеанса


int main ()
{
    printf("Start client.... \n");
    char buf[1000];
    init_UDP();
    //sound_set_eth();
    int len = sizeof(buf);
    int format,nchans,rate;
    //short buf[1000];
    int res,res2,audio_fd;
    printf("SETTINGS\n\r"); 
    audio_fd = open(AUDIO_DEVICE, O_RDWR, 0);
    if(audio_fd<0) { printf("no dsp\n\r") ;exit(0); }
    format = AFMT_S8;
    ioctl(audio_fd, SNDCTL_DSP_SETFMT, &format);
    nchans = 1;
    ioctl(audio_fd, SNDCTL_DSP_CHANNELS, &nchans);
    rate = 8000;
    ioctl(audio_fd, SNDCTL_DSP_SPEED, &rate);
    printf("Recording mode: 1000 samples\n\r");

    for(;;)
    {
        int bytes_read = recvfrom(udp_socket, (char *)buf, MAXLINE,  
                            0, (struct sockaddr *) &adr, 
                            &len); 
        //printf("%hn\n", &buf[0]);
        if ((buf[0] == 82) && (buf[1] == 48)){ 

            res2=write(audio_fd,&buf[7],bytes_read - 7);    
            
        }
    }
    close(udp_socket);
    return(0);
}

void sound_set_eth(void)
{
    int format,nchans,rate;
    short buf[1000];
    int res,res2,audio_fd;
    printf("SETTINGS\n\r"); 
    audio_fd = open(AUDIO_DEVICE, O_RDWR, 0);
    if(audio_fd<0) { printf("no dsp\n\r") ;exit(0); }
    format = AFMT_S16_LE;
    ioctl(audio_fd, SNDCTL_DSP_SETFMT, &format);
    nchans = 1;
    ioctl(audio_fd, SNDCTL_DSP_CHANNELS, &nchans);
    rate = 8000;
    ioctl(audio_fd, SNDCTL_DSP_SPEED, &rate);
    printf("Recording mode: 1000 samples\n\r"); 
    res=read(audio_fd,buf,sizeof(buf));
    printf("Playing %d samples\n\r",res); 
    res2=write(audio_fd,buf,res);
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
    adr.sin_addr.s_addr = INADDR_ANY;
    adr.sin_port = htons(3301); // принимаем на этот порт
    //inet_aton("127.0.0.1" , &adr.sin_addr);
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
