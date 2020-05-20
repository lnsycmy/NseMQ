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
        ERR_NO_ERROR = 0,
        /* producer error code. */
        ERR_FAIL_INIT_SERVERS = -1,
        ERR_FAIL_INIT_DR_CB = -2,
        ERR_FAIL_CREATE_PRODUCER = -3,
        ERR_SEND_MSG_EMPTY = -4,
        ERR_SEND_FAIL = -5,
        ERR_SEND_MSG_TOO_LARGE = -6,
        ERR_SEND_QUEUE_FULL = -7,
        ERR_SEND_UNKNOWN_TOPIC = -8,
        /* consumer error code */
        ERR_CONF_BROKER_ADDR = -11,          // failed to set up broker address.
        ERR_CONF_DR_CB = -12,                // failed to set up
        ERR_CREATE_CONSUMER = -13,           // failed to create consumer.
        ERR_CREATE_COMPLEX_CONSUMER = -14,   // failed to create complex consumer.
        ERR_SUBS_FAIL_CREATE_TOPIC = -15,
        ERR_SUBS_FAIL_START_CONSUMER = -16,
        ERR_SUBS_FAIL_BIND_CALLBACK = -17,
        ERR_SUBS_TOPIC_EMPTY = -24,
        ERR_UNSUBS_TOPIC_EMPTY = -18,
        ERR_UNSUBS_TOPIC_NO_FIND = -19,
        ERR_UNSUNS_TOPIC_FAILED = -20,
        ERR_START_TOPIC_EMPTY = -21,
        ERR_START_CREATE_THREAD = -22,
        ERR_FAIL_CONNECT_BROKER = -23,


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
