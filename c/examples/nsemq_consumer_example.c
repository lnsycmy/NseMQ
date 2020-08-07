#include <stdio.h>
#include <string.h>
#include <librdkafka/rdkafka.h>
#include "../src/nsemq_consumer.h"

static void msg_callback1 (rd_kafka_message_t *rkmessage,
                         void *opaque) {
    printf("enter msg_callback1\n");
    printf("received data:%s \n", rkmessage->payload);
    // rd_kafka_message_destroy(rkmessage);
}

static void msg_callback2 (rd_kafka_message_t *rkmessage,
                           void *opaque) {
    printf("enter msg_callback2\n");
    printf("received data:%s \n", rkmessage->payload);
    // rd_kafka_message_destroy(rkmessage);
}

int main(){
    printf("enter main();\n");
    //init("localhost:9092");
    //subscribe("test",msg_callback1);
    //subscribe("test1",msg_callback2);
    /*TopicList topic_array = NULL;
    subscription(&topic_array);
    TopicList curNode = topic_array;
    while (curNode) {
        printf("%s ", curNode->topic_name);
        curNode = curNode->next;
    }
    printf("\n");*/
    //start();
}
