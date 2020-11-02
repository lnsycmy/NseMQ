#ifndef NSEMQ_CONSUMER_H_
#define NSEMQ_CONSUMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include "utils/uuid4.h"
#include "utils/list.h"
#include "pthread/pthread.h"
#include "nsemq_base.h"
/*
 * start consuming from partition
 * RD_KAFKA_OFFSET_BEGINNING/RD_KAFKA_OFFSET_END/RD_KAFKA_OFFSET_STORED
 */
// #define NSEMQ_DEFAULT_OFFSET RD_KAFKA_OFFSET_STORED
#define NSEMQ_DEFAULT_OFFSET RD_KAFKA_OFFSET_END
#define NSEMQ_DEFAULT_PARTITION 0
#define NSEMQ_MAX_FLUSH_TIME 1000

#define nsemq_consumer_subscribe(topic, type, cb_func)  \
        nsemq_consumer_subscribe_internal(topic, #type, type##_deserialize, cb_func);

ErrorCode NSEMQ_API nsemq_consumer_init(const char *broker_addr);   // initialize consumer, and return nsemq handle.
ErrorCode NSEMQ_API nsemq_consumer_subscribe_internal(const char *topic_name,   // subscribe topic and bind consume callback
                                                      const char *data_type,
                                                      deserialize_func d_fun,
                                                      void (*consume_callback)(void *, char *, char *));
ErrorCode NSEMQ_API nsemq_consumer_unsubscribe(const char *topic);  // unsubscribe topic
ErrorCode NSEMQ_API nsemq_consumer_get_subscriptions(list_t *topic_list);// get subscribed topic names and save list.
ErrorCode NSEMQ_API nsemq_consumer_start(int async);                // start to consume message from broker.
ErrorCode NSEMQ_API nsemq_consumer_stop();                          // stop the consumer.
ErrorCode NSEMQ_API nsemq_consumer_close();                         // close the consumer.



#ifdef __cplusplus
}
#endif
#endif //NSEMQ_CONSUMER_H_
