#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "zmq.h"
#include <zlib.h>
#include <assert.h>


int main () 
{
    uLongf decompressed_size = 8192;
	void* context = zmq_ctx_new();
	printf("Client Starting….\n");
	char ucomp_buff[8102];
	void* request = zmq_socket(context, ZMQ_SUB); assert(request);
	int conn = zmq_connect(request, "tcp://localhost:4040");
	conn = zmq_setsockopt(request, ZMQ_SUBSCRIBE, "EN", 2);
    conn = zmq_setsockopt(request, ZMQ_SUBSCRIBE, "x", 1);
	for(;;) 
	{
		zmq_msg_t msg;
        int rc = zmq_msg_init (&msg); 
        rc = zmq_msg_recv(&msg, request, 0);
        int length = zmq_msg_size(&msg);
		char* value = malloc(length + 1);

        memcpy(value, zmq_msg_data(&msg), length);
        int res;
        if (value[0] == 120){
            res = uncompress((Bytef *)ucomp_buff, &decompressed_size,
                      value, length);
            if(res != Z_OK)
            {
                fprintf(stderr, "uncompress(...) failed, res = %d\n", res);
            }        
        }

        zmq_msg_close(&msg);
        if ((res == Z_OK) && (length != 0)) printf("------------------after uncompressed------------------- \n%s\n",
                                                   ucomp_buff);
        else if (length != 0)
        {
            printf("------------------original without compress----------\n%s\n", value);
        }
		free(value);
        res = -1;
	}
   
	zmq_close(request);
	zmq_ctx_destroy(context);
	
	return 0;
}