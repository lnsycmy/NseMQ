#ifndef NSEMQ_C_NSEMQ_BASE_H
#define NSEMQ_C_NSEMQ_BASE_H

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

#define TRUE 1
#define FALSE 0

typedef int BOOL;
typedef void* (*deserialize_func)(void*);

typedef NSEMQ_API enum{
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

    /* general error code */
    ERR_FAIL_CONNECT_BROKER = -100,    // failed to connect broker.
} ErrorCode;

/*** Value of running status ***/
typedef NSEMQ_API enum {
    NO_INIT = -1,
    INIT_STATUS = 0,
    START_STATUS = 1,
    CLOSE_STATUS = 2,
} RunStatus;

/*** Basic types of data ***/
typedef NSEMQ_API struct {
    serialize_fn serialize;
    get_size_fn  get_size;
    get_type_fn  get_type;
    destroy_fn   destroy;
} BaseType;

/*** topic map item ***/
typedef struct {
    rd_kafka_topic_t *topic_object;     // topic object
    void* (*deserialize_func)(void*);   // deserialize function
    void (*consume_callback)(void *, char*, char *);  // consumer callback function.
} TopicItem;
typedef map_t(TopicItem) topic_map_t;

/*** encoder and decoder function ***/
int nsemq_encode(void *msg_struct, char **msg_buf, char **msg_type);
void* nsemq_decode(char *msg_buf, int buf_size, deserialize_func d_func);

/*** consumer and deliver report callback function ***/
void nsemq_consume_callback(rd_kafka_message_t *rkmessage, void *opaque);
void nsemq_produce_callback(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque);

// print the log
void nsemq_write_error(const rd_kafka_t *rk, char *errstr);
void nsemq_write_debug(const rd_kafka_t *rk, char *debugstr);
void nsemq_write_info(const rd_kafka_t *rk, char *infostr);
BOOL nsemq_judge_connect(rd_kafka_t *handle);

#ifdef __cplusplus
}      /* extern "C" */
#endif
#endif //NSEMQ_C_NSEMQ_BASE_H
