#ifndef NSEMQ_C_NSEMQ_CONSUMER_H
#define NSEMQ_C_NSEMQ_CONSUMER_H

#include <stdio.h>
#include <pthread/pthread.h>
#include "nsemq_base.h"

/*
 * Start consuming from partition
 * RD_KAFKA_OFFSET_BEGINNING/RD_KAFKA_OFFSET_END/RD_KAFKA_OFFSET_STORED
 */
#define NSEMQ_DEFAULT_OFFSET RD_KAFKA_OFFSET_STORED
#define NSEMQ_DEFAULT_PARTITION 0
#define MAX_THREAD_NUM 128

struct TopicNode{
    char * topic_name;      // topic name
    rd_kafka_topic_t *topic_object;  // topic object
    void (*consume_callback)(rd_kafka_message_t *, void *);  // consumer callback function.
    struct TopicNode * next;
};
typedef struct TopicNode *TopicList;

/* consumer function */
ErrorCode nsemq_consumer_init(const char * broker_addr);
ErrorCode nsemq_consumer_subscribe(const char *topic_name, void *consume_callback);
ErrorCode nsemq_consumer_unSubscribe(const char *topic_name);

ErrorCode nsemq_consumer_subscription(TopicList *topic_array);
ErrorCode nsemq_consumer_start();           // start to consume message from broker.
ErrorCode nsemq_consumer_poll();            // polled to call the topic consume callback.
ErrorCode nsemq_consumer_close();           // close the consumer.

#endif //NSEMQ_C_NSEMQ_CONSUMER_H
