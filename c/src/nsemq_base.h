#ifndef NSEMQ_C_NSEMQ_BASE_H
#define NSEMQ_C_NSEMQ_BASE_H

#include <stdio.h>
#include <string.h>
#include <librdkafka/rdkafka.h>
#include "kaa_common_schema.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef void* (*deserialize_func)(void*);

#define TRUE 1
#define FALSE 0
typedef int BOOL;

typedef  enum{
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
typedef enum {
    NO_INIT = -1,
    INIT_STATUS = 0,
    START_STATUS = 1,
    CLOSE_STATUS = 2,
} RunStatus;

/*** Basic types of data ***/
typedef struct {
    serialize_fn serialize;
    get_size_fn  get_size;
    get_type_fn  get_type;
    destroy_fn   destroy;
} BaseType;

/*** topic list ***/
struct TopicNode {
    char *topic_name;      // topic name
    rd_kafka_topic_t *topic_object;  // topic object
    void* (*deserialize_func)(void*);   // deserialize function
    void (*consume_callback)(void *, char*, char *);  // consumer callback function.
    struct TopicNode *next;
};
typedef struct TopicNode *TopicList;

/*** list function ***/
void insert_list(TopicList *topic_list,
                 const char *topic_name,
                 rd_kafka_topic_t *topic_object,
                 void* (*deserialize_func)(void*),
                 void (*consume_callback)(void *, char*, char *));
void display_list(TopicList topic_list);
void delete_item(TopicList *topic_list,const char *topic_name);
void clear_list(TopicList *topic_list);
TopicList find_item(TopicList topic_list,const char *topic_name);

/*** encoder and decoder function ***/
int nsemq_encode(void *msg_struct, char **msg_buf, char **msg_type);
void* nsemq_decode(char *msg_buf, int buf_size, deserialize_func d_func);


/*** consumer and deliver report callback function ***/
void nsemq_consume_callback(rd_kafka_message_t *rkmessage, void *opaque);
void nsemq_produce_callback();

// print the log
void nsemq_write_error(char *errstr);
void nsemq_write_info(char *infostr);
BOOL nsemq_judge_connect(rd_kafka_t *handle);

#ifdef __cplusplus
}      /* extern "C" */
#endif
#endif //NSEMQ_C_NSEMQ_BASE_H
