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
#include <pthread.h>

#define AUDIO_DEVICE "/dev/dsp"
#define pakage 6
#define frame 167

#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define SUCCESS        0


void init_UDP(void);

int udp_socket; // Сокет UDP
struct sockaddr_in adr; // Адрес для бродкаста в рамках текущего сеанса


typedef struct someArgs_tag {
    int audio_fd;
    unsigned char *per_buf;
    int n;
} someArgs_t;


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

void* write_to_audio(void *args)
{
someArgs_t *arg = (someArgs_t*) args;
int audio_fd = arg->audio_fd;
int n = arg->n;
write(audio_fd,arg->per_buf,n - ((pakage + 1) * 4)); //пушаем в звуковуху с 7 элемента (сами данные) 
pthread_exit(0);
}


int main ()
{
    printf("Start client.... \n");


    pthread_t thread;
    int status;
    int status_addr;
    someArgs_t args;

    short int Size_for = 1 + (frame * pakage);
    unsigned char buf[1200];
    unsigned char temp_buf[frame];
    unsigned char buffer[frame + 3];
    init_UDP();
    int len = sizeof(buf);
    int len_bufrecv = sizeof(buffer);
    int bytes_read;
    args.audio_fd = set_audio_dev();


    for(;;)
    {
        memset(&buffer, 0, sizeof(buffer)); 
        memset(&buf, 0, sizeof(len)); 
        int n = 0;
        for(int co = 0; co < Size_for;){
            bytes_read = recvfrom(udp_socket, buffer, sizeof(buf),  
                                0, (struct sockaddr *) &adr, 
                                &len_bufrecv); 
            if ((buffer[0] == 82) && (buffer[1] == 48)){
                co = co + bytes_read;
                for (int co1 = 3;(co - (3 * (co / frame)) > n); n++, co1++){
                    buf[n]= buffer[co1];  // в buf складируется примерно 7 частей 
                }
            }
        }
        //-------------------------jitter buf correct begin------------------------------------
        for (size_t i = 0; i < pakage - 1; i++) //пузырьком сравниваем метки времени
        {
            for (size_t j = (pakage - 1) * (frame - 3); j > i;)
            {
                if (buf[j - (frame - 3)] > buf[j]){ // если текущий элемент меньше предыдущего

                    for (size_t k = 0; k < (frame - 3); k++) // запомнили текущий
                    {
                        temp_buf[k] = buf[j + k];
                    }
                    

                    for (size_t k = 0; k < (frame - 3); k++)
                    {
                        buf[j - (frame - 3) + k] = buf[j + k];
                    }
                    

                    for(size_t k = 0; k < (frame - 3); k++)
                    {
                        buf[j + k] = temp_buf[k];
                    }

                }
                j = j - (frame - 3);
                
            }
        
           
        }
        //-------------------------jitter buf correct end------------------------------------
        for (int col = 0;col < pakage + 1;col++){

            for (size_t k = 0; k < (frame - 7); k++) // запомнили текущий
            {
                temp_buf[k] = buf[((col * (frame - 3)) + k + 4)];
            }
            for (int j = 0;(frame - 7) > j; j++){
                buf[j + (col * (frame - 7))] = temp_buf[j];  // в buf складируется примерно 6 частей 
            }
        }
        args.n = n;
        args.per_buf = &buf[0];
        status = pthread_create(&thread, NULL, write_to_audio, (void*) &args);
        if (status != 0) {
        printf("main error: can't create thread, status = %d\n", status);
        exit(ERROR_CREATE_THREAD);
        }

        
        status = pthread_join(thread, (void**)&status_addr);
        if (status != SUCCESS) {
        printf("main error: can't join thread, status = %d\n", status);
        exit(ERROR_JOIN_THREAD);
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
