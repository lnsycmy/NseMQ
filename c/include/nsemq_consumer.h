#ifndef NSEMQ_CONSUMER_H_
#define NSEMQ_CONSUMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include "utils/uuid4.h"
#include "utils/list.h"
#include "nsemq_base.h"
// include "pthread/pthread.h" in VS and mingw32
#if (defined(_MSC_VER) || defined(__MINGW32_VERSION))
#include "pthread/pthread.h"
#else
#include <pthread.h>
#endif // _MSC_VER

// #define NSEMQ_DEFAULT_OFFSET RD_KAFKA_OFFSET_STORED
// #define RD_KAFKA_OFFSET_BEGINNING/RD_KAFKA_OFFSET_END/RD_KAFKA_OFFSET_STORED
#define NSEMQ_DEFAULT_OFFSET RD_KAFKA_OFFSET_END
#define NSEMQ_DEFAULT_PARTITION 0
#define NSEMQ_MAX_FLUSH_TIME 1000

/**
 * Subscribe topic, bind consume message type and consume callback function.
 * it can be called after init(), start() and stop(), but must be before close().
 * @param topic: topic to be subscribed, the type is char*, i.e. "test".
 * @param type: bind the data type of consumption, the type is macro, i.e. nse_cpx.
 * @param cb_func: consume callback function pointer, i.e. msg_callback.
 */
#define nsemq_consumer_subscribe(topic, type, cb_func)  \
        nsemq_consumer_subscribe_internal(topic, #type, (ds_msg_func)(type##_deserialize), cb_func);

/**
 * Initialize the consumer by broker address, the consumer handle created internally.
 * @param broker_addr: broker ip address and port, i.e. "localhost:9092".
 * @return ErrorCode: error code, used to locate root cause of the error, omitted below.
 */
ErrorCode NSEMQ_API nsemq_consumer_init(const char *broker_addr);

/**
 * [internal] subscribe topic function, called by nsemq_consumer_subscribe() internally.
 * @param topic_name: topic to be subscribed, the type is char*, i.e. "test".
 * @param data_type: bind the data type of consumption, i.e. "nse_cpx".
 * @param d_fun: bind the deserialize function pointer of data type, i.e. nse_cpx_deserialize.
 * @param consume_callback: bind the consume callback function pointer, i.e. msg_callback.
 */
ErrorCode NSEMQ_API
nsemq_consumer_subscribe_internal(const char *topic_name,
                                  const char *data_type,
                                  ds_msg_func d_fun,
                                  msg_cb_func consume_callback);

/**
 * Unsubscribe one topic, stop consuming this topic if start() have been called.
 * it can be called after subscribe(), start() and stop(), but must be before close().
 * @param topic: topic will be unsubscribed.
 */
ErrorCode NSEMQ_API nsemq_consumer_unsubscribe(const char *topic);

/**
 * Get all subscribed topic names and save it to topic_list.
 * @param topic_list: topic list, the type is "list_t", which needs to be created by user.
 */
ErrorCode NSEMQ_API nsemq_consumer_subscriptions(list_t *topic_list);

/**
 * For the subscribed topics, start to consume messages from the broker server.
 * @param async: async tags represent non-blocking consumption, and sync tags represent blocking consumption.
 */
ErrorCode NSEMQ_API nsemq_consumer_start(int async);

/**
 * stop consuming message from the broker server, and keep subscribed topics.
 * it can call start() to restart consumption.
 */
ErrorCode NSEMQ_API nsemq_consumer_stop();

/**
 * close the consumer handle, and destroy the allocated memory.
 * it can call stop() automatically if not called before.
 */
ErrorCode NSEMQ_API nsemq_consumer_close();

#ifdef __cplusplus
}
#endif
#endif //NSEMQ_CONSUMER_H_
