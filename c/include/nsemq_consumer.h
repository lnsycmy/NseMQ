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
 * start consuming from partition
 * RD_KAFKA_OFFSET_BEGINNING/RD_KAFKA_OFFSET_END/RD_KAFKA_OFFSET_STORED
 */
// #define NSEMQ_DEFAULT_OFFSET RD_KAFKA_OFFSET_STORED
#define NSEMQ_DEFAULT_OFFSET RD_KAFKA_OFFSET_END
#define NSEMQ_DEFAULT_PARTITION 0

#define nsemq_consumer_subscribe(topic, type, cb_func)  \
        nsemq_consumer_subscribe_internal(topic, #type, type##_deserialize, cb_func);

/* external consumer function */
ErrorCode NSEMQ_API nsemq_consumer_init(const char *broker_addr);       // initialize consumer
ErrorCode NSEMQ_API nsemq_consumer_subscribe_internal(const char *topic_name, // subscribe topic and bind consume callback
                                                      const char *data_type,
                                                      deserialize_func d_fun,
                                                      void (*consume_callback)(void *, char *, char *));
ErrorCode NSEMQ_API nsemq_consumer_unsubscribe(const char *topic_name); // unsubscribe topic
ErrorCode NSEMQ_API nsemq_consumer_subscription(char **topic_array, int *topic_count);  // get subscribed topic names.
ErrorCode NSEMQ_API nsemq_consumer_start();           // start to consume message from broker.
ErrorCode NSEMQ_API nsemq_consumer_close();           // close the consumer.

#ifdef __cplusplus
}
#endif
#endif //NSEMQ_C_NSEMQ_CONSUMER_H
