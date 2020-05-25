//
// Created by cmy on 2020/5/1.
//
#pragma once
#include "student.hh"
#include "cpx.hh"
#include "../src/NseMqProducer.h"
#include <windows.h>
/* the delivery report callback.
 * This callback will be called once per message to inform
 * the application if delivery succeeded or failed.
 * The callback is only triggered from ::poll() and ::flush().
 * IMPORTANT:
 * Make sure the DeliveryReport instance outlives the Producer object,
 * either by putting it on the heap or as in this case as a stack variable
 * that will NOT go out of scope for the duration of the Producer object.
 */
class ProducerCallback : public RdKafka::DeliveryReportCb {
public:
    void dr_cb(RdKafka::Message &message){
        // If message.err() is non-zero the message delivery failed permanently for the message.
        if (message.err()){
            std::cerr << "Message delivery failed: " << message.errstr() << std::endl;
        }else{
            std::cerr << "Message delivered to topic " << message.topic_name() <<
                      " [" << message.partition() << "] at offset " <<
                      message.offset() << std::endl;
        }
    }
};
int main(){
    std::string broker_addr = "localhost:9092";
    std::string topic_name = "test";
    std::string topic_name2 = "test2";

    NseMqProducer producer;
    ProducerCallback *producer_cb = new ProducerCallback();

    producer.init(broker_addr, producer_cb);

    NseMQ::student s1;
    s1.name = "cmy";
    s1.sex = "boy";
    s1.age = 24;
    if(producer.produce<NseMQ::student>(s1, topic_name) != NseMQ::ERR_NO_ERROR){
        std::cout << "failed produce student in main()"<< std::endl;
    }
    NseMQ::cpx c1;
    c1.re = 1.2;
    c1.im = 2.4;
    if(producer.produce<NseMQ::cpx>(c1, topic_name2) != NseMQ::ERR_NO_ERROR){
        std::cout << "failed cpx student in main()"<< std::endl;
    }
    return 0;
}