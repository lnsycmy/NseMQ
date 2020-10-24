#ifndef NSEMQ_C_NSEMQ_CONSUMER_H
#define NSEMQ_C_NSEMQ_CONSUMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include "utils/uuid4.h"
#include "pthread/pthread.h"
#include "nsemq_base.h"
/*
 * Start consuming from partition
 * RD_KAFKA_OFFSET_BEGINNING/RD_KAFKA_OFFSET_END/RD_KAFKA_OFFSET_STORED
 */
// #define NSEMQ_DEFAULT_OFFSET RD_KAFKA_OFFSET_STORED
#define NSEMQ_DEFAULT_OFFSET RD_KAFKA_OFFSET_END
#define NSEMQ_DEFAULT_PARTITION 0

#define NSEMQ_CONSUMER_SUBSCRIBE(topic, type, cb_func)  \
    nsemq_consumer_subscribe(topic, type##_deserialize, cb_func);

/* consumer function */
ErrorCode NSEMQ_API nsemq_consumer_init(const char *broker_addr);

ErrorCode NSEMQ_API nsemq_consumer_subscribe(const char *topic_name,
                                   deserialize_func d_fun,
                                   void (*consume_callback)(void *, char *, char *));

ErrorCode NSEMQ_API nsemq_consumer_unSubscribe(const char *topic_name);

ErrorCode NSEMQ_API nsemq_consumer_subscription(TopicList *topic_array);

ErrorCode NSEMQ_API nsemq_consumer_start();           // start to consume message from broker.
ErrorCode NSEMQ_API nsemq_consumer_poll();            // polled to call the topic consume callback.
ErrorCode NSEMQ_API nsemq_consumer_close();           // close the consumer.

#ifdef __cplusplus
}
#endif
#endif //NSEMQ_C_NSEMQ_CONSUMER_H
