#ifndef NSEMQ_NSEMQHANDLE_H_
#define NSEMQ_NSEMQHANDLE_H_
/**
 * handle header file.
 * @author cmy
 * @date 2020/4/26.
 */
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "librdkafka/rdkafkacpp.h"

namespace NseMQ{
    enum ErrorCode{
        ERR_NO_ERROR = 0,                   // execution succeed, no-error.
        /* producer error code. */
        ERR_P_INIT_BROKER_ADDRESS = -1,     // failed to set up broker address.
        ERR_P_INIT_DR_CALLBACK = -2,        // failed to set up delivery report callback.
        ERR_P_CREATE_PRODUCER = -3,         // failed to create kafka producer.
        ERR_P_SEND_MSG_EMPTY = -4,          // send message is empty.
        ERR_P_SEND_QUEUE_FULL = -5,         // send message queue is full.
        ERR_P_SEND_MSG_TOO_LARGE = -6,      // send message is to large.
        ERR_P_SEND_UNKNOWN_TOPIC = -7,      // send message but unknown topic.
        ERR_P_SEND_FAIL = -8,               // failed to send message with other error.
        /* consumer error code */
        ERR_C_INIT_BROKER_ADDRESS = -21,    // failed to set up broker address.
        ERR_C_CREATE_CONSUMER = -22,        // failed to create consumer.
        ERR_C_SUBS_CREATE_TOPIC = -23,      // failed to create topic with topic name.
        ERR_C_SUBS_BROKER_TOPIC = -24,      // failed to subscribe topic from broker.
        ERR_C_SUBS_LOCAL_TOPIC = -25,       // failed to subscribe topic from local.
        ERR_C_UNSUBS_TOPIC_NO_FIND = -26,   // failed to find topic from local.
        ERR_C_UNSUNS_BROKER_TOPIC = -27,    // failed to unsubscribe topic from broker.
        ERR_C_SUBS_TOPIC_EMPTY = -28,       // failed to get topic list.
        ERR_C_POLL_TOPIC_EMPTY = -29,       // failed to find topic as no have subscribed topic.
        ERR_C_START_CREATE_THREAD = -30,    // failed to create thread when called start().

        /* general error code */
        ERR_FAIL_CONNECT_BROKER = -100,    // failed to connect broker.


    };
}
class NseMqHandle{
public:
    NseMqHandle();
    ~NseMqHandle();
    // test connection with broker.
    bool judgeConnectionImpl(RdKafka::Handle *handle);
    // get all topics from broker.
    void getBrokerTopicsImpl(std::vector<std::string> &topics, RdKafka::Handle *handle);
    // write error log.
    void writeErrorLogImpl(std::string err_str, std::string write_object);
};

#endif // NSEMQ_NSEMQHANDLE_H_
