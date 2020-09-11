//
// Created by cmy on 2020/5/1.
//
#pragma once
#include "student.hh"
#include "cpx.hh"
#include "../src/NseMqProducer.h"
/* the delivery report callback.
 * This callback will be called once per message to inform
 * the application if delivery succeeded or failed.
 * The callback is only triggered from ::poll() and ::flush().
 * IMPORTANT:
 * Make sure the DeliveryReport instance outlives the Producer object,
 * either by putting it on the heap or as in this case as a stack variable
 * that will NOT go out of scope for the duration of the Producer object.
 */
/*class ProducerCallback : NseMqProducerCallback {
public:
    void dr_msg_cb (rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque){
        if (rkmessage->err){
            fprintf(stderr, "%% Message delivery failed: %s\n",
                    rd_kafka_err2str(rkmessage->err));
        }else{
            std::cout << "%% Message delivered " << rkmessage->len
                      << " bytes, partition " << rkmessage->partition << std::endl;
        }
    }
};*/

void dr_msg_cb (rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque) {
    if (rkmessage->err){
        fprintf(stderr, "%% Message delivery failed: %s\n",
                rd_kafka_err2str(rkmessage->err));
    }else{
        std::cout << "%% Message delivered " << rkmessage->len
                  << " bytes, partition " << rkmessage->partition << std::endl;
    }
}
int main(){
    std::string broker_addr = "localhost:9092";
    std::string topic_name = "test";
    std::string topic_name2 = "test2";

    NseMqProducer producer;

    if(producer.init(broker_addr,dr_msg_cb) != NseMQ::ERR_NO_ERROR){
        return -1;
    }

    NseMQ::student s1;
    s1.name = "cmy";
    s1.sex = "boy";
    s1.age = 24;
    if(producer.produce<NseMQ::student>(s1, topic_name) != NseMQ::ERR_NO_ERROR){
        std::cout << "failed produce student in main()" << std::endl;
    }
    NseMQ::cpx c1;
    c1.re = 1.2;
    c1.im = 2.4;
    if(producer.produce<NseMQ::cpx>(c1, topic_name2) != NseMQ::ERR_NO_ERROR){
        std::cout << "failed cpx student in main()"<< std::endl;
    }
    return 0;
}