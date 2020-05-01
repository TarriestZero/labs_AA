#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <zlib.h>
#include <string.h>
#include <assert.h>
#include <zmq.h>


char* Shakespeare1="EN: From fairest creatures we desire increase,\n\rThat thereby beauty\'s rose might never die,\n\rBut as the riper should by timedecease,\n\rHis tender heir might bear his memory:\n\rBut thou contracted to thine own bright eyes,\n\rFeed\'st thy light\'s flame with selfsubstantial fuel,\n\rMaking a famine where abundance lies,Thy self thy foe, to thy sweet self too cruel:Thou that art now the world's fresh ornament,\n\rAnd only herald to the gaudy spring,\n\rWithin thine own bud buriest thy content,\n\rAnd, tender churl,\n\r mak\'st waste in niggarding:\n\rPity the world, or else this glutton be,\n\rTo eat the world\'s due,\n\r by the grave and thee.";

char* ShakespeareMarshak1="RU: Мы урожая ждем от лучших лоз,\n\rЧтоб красота жила, не увядая.\n\rПусть вянут лепестки созревших роз,\n\rХранит их память роза молодая.\n\r\n\rА ты, в свою влюбленный красоту,\n\rВсе лучшие ей отдавая соки,\n\rОбилье превращаешь в нищету, -\n\rСвой злейший враг, бездушный и жестокий.\n\r\n\rТы - украшенье нынешнего дня,\n\rНедолговременной весны глашатай, -\n\rГрядущее в зачатке хороня,\n\rСоединяешь скаредность с растратой.\n\r\n\rЖалея мир, земле не предавай\n\rГрядущих лет прекрасный урожай!\n\r";

int main (void) {
    char message[8102];
    zmq_msg_t msg;  
    struct timeval tv;
    __timezone_ptr_t tz;
    int rc,ret;
    
    void *context;
    printf("1");
    context=zmq_ctx_new(); if(context==NULL) exit(0); // создали контекст
    
    void *publisher = zmq_socket (context, ZMQ_PUB);assert (context); // создали 0mq гнездо- publisher 
    
    
    int conn = zmq_bind(publisher, "tcp://*:4040");
    

    while (1) {
        gettimeofday(&tv,tz);
        if(tv.tv_sec%2==0){
            if(tv.tv_sec%5==0){ //Один раз в 5 секунд выполняется отправка в канал оригинала текста
                
                sprintf(message,"%s %d\n",Shakespeare1,(int) tv.tv_sec ); // Орининал Шекспира английский текст
                rc = zmq_msg_init_size (&msg, strlen(message)); assert (rc == 0); // подготовили объект
                //rc = zmq_msg_init_data (&msg, message,strlen(message), NULL, NULL); assert (rc == 0); // заполнили данными
                memcpy(zmq_msg_data(&msg), message, strlen(message));
            }
            else { // Сжатый текст
                unsigned long Zlen=8192; // максимальный размер буфера,чтобы не случилось переполнение
                char prebuf[8192];
                sprintf(prebuf,"%s %d\n",ShakespeareMarshak1,(int) tv.tv_sec ); 
                ret=compress((Byte *) message,&Zlen,(const Bytef *)prebuf,strlen(prebuf));
                // Оригинал Шекспира - сжатый текст
                rc = zmq_msg_init_size (&msg, Zlen); 
                //rc = zmq_msg_init_data (&msg, message,Zlen , NULL, NULL); assert (rc == 0);          
                memcpy(zmq_msg_data(&msg), message, Zlen);
            }
        }       
        else // все нечетные секунды опправляется перевод текста в переводе С Маршака
        {
            sprintf(message,"%s %d\n",ShakespeareMarshak1,(int) tv.tv_sec );// Перевод Шекспира - русский текст
            rc = zmq_msg_init_size (&msg, strlen(message)); assert (rc == 0);
            //rc = zmq_msg_init_data (&msg, message,strlen(message), NULL, NULL); assert (rc == 0);
            memcpy(zmq_msg_data(&msg), message, strlen(message));
        }        
        
        /* Send the message to the socket */
        // v2.2-rem    rc = zmq_send (publisher, &msg, 0); assert (rc == 0);
        zmq_msg_send(&msg,publisher,0);
        zmq_msg_close(&msg);
        sleep (1);
    }
    // We never get here, but clean up anyhow
    zmq_msg_close (publisher);
    // v2.2-rem    zmq_destroy(context);
    zmq_ctx_destroy(context);
    return 0;
}