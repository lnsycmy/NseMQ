#ifndef NSEMQ_NSEMQCONSUMERCALLBACK_H
#define NSEMQ_NSEMQCONSUMERCALLBACK_H
//
// Created by cmy on 2020/5/14.
//

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <typeinfo>
#include "NseMqBase.h"
#include "librdkafka/rdkafkacpp.h"

template<class T>
class NseMqConsumerCallback : public RdKafka::ConsumeCb, public NseMqBase{
public:
    // consume callback function which need to implement by user.
    virtual void consume_callback(T &t) = 0;

    // deal with receive message and call the consume_callback(T &t).
    bool msg_package(RdKafka::Message &msg){
        // compare the type of T and message.
        const RdKafka::Headers *headers = msg.headers();
        if(headers){
            const RdKafka::Headers::Header hdr = headers->get_all()[0];
            if(NULL != hdr.value()){
                const char *msg_type = (const char *)hdr.value();
                if(strcmp(msg_type, typeid(T).name()) != 0){
                    std::string err_str(typeid(T).name());
                    std::string msg_type_str(msg_type);
                    err_str += " callback function receive a different type message:";
                    err_str += msg_type_str;
                    this->writeErrorLog(err_str);
                    return false;
                }
            }
        }
        // get message length.
        size_t msg_len = msg.len();
        // serialize the msg to t.
        T t;
        this->decode(t, static_cast<const unsigned char *>(msg.payload()), msg_len);
        consume_callback(t);
        return true;
    }
    // implement rdkafka consume callback function.
    void consume_cb(RdKafka::Message &msg, void *opaque){
        switch (msg.err()) {
            case RdKafka::ERR_NO_ERROR:
                // Deal message
                this->msg_package(msg);
                break;
            case RdKafka::ERR__TIMED_OUT:
                this->writeErrorLog("time out" + msg.errstr());
                break;
            case RdKafka::ERR__PARTITION_EOF:
                // Last message
                break;
            case RdKafka::ERR__UNKNOWN_TOPIC:
            case RdKafka::ERR__UNKNOWN_PARTITION:
                this->writeErrorLog("Consume failed: " + msg.errstr());
                break;
        }
    }

    void writeErrorLog(std::string err_str){
        this->writeErrorLogImpl(err_str, "ConsumerCallback");
    }
};

#endif //NSEMQ_NSEMQCONSUMERCALLBACK_H
