#ifndef NSEMQ_C_NSEMQ_CONSUMER_H
#define NSEMQ_C_NSEMQ_CONSUMER_H

#include <stdio.h>
#include <string.h>
#include <librdkafka/rdkafka.h>
#include <pthread/pthread.h>
#include "nsemq_base.h"

#define NSEMQ_DEFAULT_OFFSET RD_KAFKA_OFFSET_END
#define NSEMQ_DEFAULT_PARTITION 0


struct TopicNode{
    char * topic_name;      // topic name
    rd_kafka_topic_t *topic_object;  // topic object
    void (*consume_callback)(rd_kafka_message_t *, void *);  // consumer callback function.
    struct TopicNode * next;
};
typedef struct TopicNode *TopicList;


/* consumer function */
ErrorCode init(const char * broker_addr);
ErrorCode subscribe(char *topic_name,
                    void (*consume_callback)(rd_kafka_message_t *rkmessage,void *opaque));
ErrorCode unSubscribe(char *topic_name);

ErrorCode subscription(TopicList *topic_array);
ErrorCode start();           // start to consume message from broker.
ErrorCode poll();            // polled to call the topic consume callback.
ErrorCode close();           // close the consumer.

// test2
ErrorCode subscribe2(char *topic_name);
ErrorCode start2();

#endif //NSEMQ_C_NSEMQ_CONSUMER_H
