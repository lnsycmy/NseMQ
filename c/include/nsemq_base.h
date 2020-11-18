#ifndef NSEMQ_BASE_H_
#define NSEMQ_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "librdkafka/rdkafka.h"
#include "kaa/kaa_common_schema.h"

#include "utils/map.h"

#ifdef NSEMQ_EXPORTS
#define NSEMQ_API __declspec(dllexport)
#else
#define NSEMQ_API __declspec(dllimport)
#endif
// Log level
#define LOG_EMERG   0
#define LOG_ALERT   1
#define LOG_CRIT    2
#define LOG_ERR     3
#define LOG_WARNING 4
#define LOG_NOTICE  5
#define LOG_INFO    6
#define LOG_DEBUG   7

// Consume flag
#define NSEMQ_ASYNC 1
#define NSEMQ_SYNC  0

#define TRUE        1
#define FALSE       0

typedef int BOOL;

/**
 * Used to define the deliver report callback function.
 * @param msg_topic: topic of the deliver message.
 * @param msg_data: message buffer which have been delivered.
 * @param msg_size: size of message buffer.
 */

typedef void (*dr_cb_func)(const char *msg_topic, void *msg_data, int msg_size);

/**
 * Used to define the consume callback function.
 * @param msg_data: Serialized message data, can be converted to other data types.
 * @param msg_topic: consume the topic of this message.
 * @param msg_type: message data type, i.e. "nse_cpx".
 */

typedef void (*msg_cb_func)(void *msg_data, char *msg_topic, char *msg_type);

/**
 * [internal] Used to define the deserialize message function.
 * @param reader: avro reade, used to deserialize message.
 */
typedef void *(*ds_msg_func)(avro_reader_t reader);

/* used to obtain the handle of internal producer or consumer. */
typedef rd_kafka_t nsemq_handle_t;

/* error code, returned from function, used to locate the cause of the error. */
typedef enum {
    ERR_NO_ERROR = 0,                   // execution succeed, no-error.
    /* producer error code. -1~-20 */
    ERR_P_INIT_BROKER_ADDRESS = -1,     // failed to set up broker address.
    ERR_P_INIT_DR_CALLBACK = -2,        // failed to set up delivery report callback.
    ERR_P_INIT_MULTIPLE_INIT = -3,      // failed to initialization multiple times.
    ERR_P_CREATE_PRODUCER = -4,         // failed to create kafka producer.
    ERR_P_SEND_MSG_EMPTY = -5,          // send message is empty.
    ERR_P_SEND_QUEUE_FULL = -6,         // send message queue is full.
    ERR_P_SEND_MSG_TOO_LARGE = -7,      // send message is to large.
    ERR_P_SEND_UNKNOWN_TOPIC = -8,      // send message but unknown topic.
    ERR_P_SEND_FAIL = -9,               // failed to send message with other error.
    ERR_P_RUN_STATUS = -10,              // error to call function with limit run status.
    /* consumer error code. -21~-40 */
    ERR_C_INIT_BROKER_ADDRESS = -21,    // failed to set up broker address.
    ERR_C_INIT_GROUP_ID = -22,          // failed to set up broker address.
    ERR_C_INIT_MULTIPLE_INIT = -23,     // failed to initialization multiple times.
    ERR_C_CREATE_CONSUMER = -24,        // failed to create consumer.
    ERR_C_SUBS_CREATE_TOPIC = -25,      // failed to create topic with topic name.
    ERR_C_SUBS_BROKER_TOPIC = -26,      // failed to subscribe topic from broker.
    ERR_C_SUBS_LOCAL_TOPIC = -27,       // failed to subscribe topic from local.
    ERR_C_UNSUBS_TOPIC_NO_FIND = -28,   // failed to find topic from local.
    ERR_C_UNSUNS_BROKER_TOPIC = -29,    // failed to unsubscribe topic from broker.
    ERR_C_SUBS_TOPIC_EMPTY = -30,       // failed to get topic list.
    ERR_C_POLL_TOPIC_EMPTY = -31,       // failed to find topic as no have subscribed topic.
    ERR_C_START_CREATE_THREAD = -32,    // failed to create thread when called start().
    ERR_C_RUN_STATUS = -33,             // error to call function with limit run status.
    ERR_C_GET_SUBS_MEMORY = -34,        // not enough memory allocated when acquiring subscription.
    ERR_C_STOP_CANCEL_THRED = -35,      // failed to cancel thread, the return value of pthread_cancel is not 0.

    /* general error code */
    ERR_FAIL_CONNECT_BROKER = -100,    // failed to connect broker.
} ErrorCode;

/* Value of running status */
typedef enum {
    NO_INIT = -1,       // not initialize, default status.
    INIT_STATUS = 0,    // complete initialization, after call init().
    START_STATUS = 1,   // running the main program, after call produce() or start().
    STOP_STATUS = 2,    // stop the main program in consumer, after call stop().
    CLOSE_STATUS = 3,   // close the
} RunStatus;

/* Basic types of data */
typedef struct {
    serialize_fn serialize;
    get_size_fn get_size;
    get_type_fn get_type;
    destroy_fn destroy;
} BaseType;

/* [internal] topic map item */
typedef struct {
    char *bind_data_type;               // data type, a topic bind one data type
    rd_kafka_topic_t *topic_object;     // topic object
    ds_msg_func deserialize_func;       // deserialize function, which be consistent with the data type
    msg_cb_func consume_callback;       // consumer callback function.
    int subs_status;                    // subscribe status, 1
} TopicItem;

/*** [internal] topic map composed of TopicItem ***/
typedef map_t(TopicItem) topic_map_t;

/*** [internal] encoder and decoder function ***/
int nsemq_encode(void *msg_struct, char **msg_buf, char **msg_type);

void *nsemq_decode(char *msg_buf, int buf_size, ds_msg_func deserialize_func);

/*** [internal] consumer and deliver report callback function ***/
void nsemq_consume_callback(rd_kafka_message_t *rkmessage, void *opaque);

void nsemq_produce_callback(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque);

/*** [internal] print the log to stderr ***/
void nsemq_write_error(const rd_kafka_t *rk, char *errstr);

void nsemq_write_debug(const rd_kafka_t *rk, char *debugstr);

void nsemq_write_info(const rd_kafka_t *rk, char *infostr);

/*** [internal] whether the connection is successful ***/
BOOL nsemq_judge_connect(nsemq_handle_t *handle);

#ifdef __cplusplus
}      /* extern "C" */
#endif
#endif //NSEMQ_BASE_H_
