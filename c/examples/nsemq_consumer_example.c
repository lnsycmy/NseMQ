#include <stdio.h>
#include <string.h>
#include <librdkafka/rdkafka.h>
#include "../src/nsemq_consumer.h"

static void msg_callback1 (rd_kafka_message_t *rkmessage,
                         void *opaque) {
    printf("enter msg_callback1\n");
    printf("received data:%s \n", (char *)rkmessage->payload);
    //rd_kafka_message_destroy(rkmessage);
}

static void msg_callback2 (rd_kafka_message_t *rkmessage,
                           void *opaque) {
    printf("enter msg_callback2\n");
    printf("received data:%s \n", (char *)rkmessage->payload);
    // rd_kafka_message_destroy(rkmessage);
}

int main(){
    int time_count = 0;
    printf("enter main();\n");
	// system("pause");
	if(nsemq_consumer_init("localhost:9092") != ERR_NO_ERROR) {
        return -1;
    }
    nsemq_consumer_subscribe("test",msg_callback1);
    nsemq_consumer_subscribe("test1",msg_callback2);
    nsemq_consumer_start();
    printf("reback main();\n");

    while(time_count < 10){
        Sleep(1000);
        time_count++;
    }
    nsemq_consumer_close();
    printf("main() end!\n");
}
