//
// Created by cmy on 2020/5/1.
//
#pragma once
#include <iostream>
#include <thread>
#include <map>
#include "NseMqSerializer.h"
#include "NseMqConsumer.h"
#include "student.hh"
#include "cpx.hh"
#include "NseMqConsumerCallback.h"
/*#include <windows.h>*/


template<class T>
class ConsumerCallback : public NseMqConsumerCallback<T>{
public:
    void consume_callback(NseMQ::student &t) {
        std::cout << "ConsumerCallback student" << std::endl;
        std::cout << " NseMQ::student cb name:" << t.name << std::endl;
        std::cout << " NseMQ::student cb age:" << t.age << std::endl;
        std::cout << " NseMQ::student cb sex:" << t.sex << std::endl;
    };
    void consume_callback(NseMQ::cpx &t) {
        std::cout << "ConsumerCallback cpx" << std::endl;
        std::cout << " NseMQ::cpx cb im:" << t.im << std::endl;
        std::cout << " NseMQ::cpx cb re:" << t.re << std::endl;
    };
    void consume_callback(std::string &t){
        std::cout <<t <<std::endl;
    }
};

int main(){
    std::cout << "enter main()" <<std::endl;
    NseMqConsumer consumer;
    consumer.init("localhost:9092");

    ConsumerCallback<NseMQ::student> cb1;
    ConsumerCallback<NseMQ::cpx> cb2;
    consumer.subscribe("test",cb1);
    consumer.subscribe("test2",cb2);
    /*consumer->start();
    Sleep(-1);*/
   /* Sleep(10000);
    consumer->pause();
    Sleep(10000);
    consumer->resume();
    Sleep(5000);*/

    auto threadFunction = [&consumer]() {
        while (1){
            consumer.poll();
            std::cout << "receive poll()" << std::endl;
        }
    };
    std::thread thread(threadFunction);
    thread.join();

    /* test connection */
    /*if(consumer->judgeConnection()){
        std::cout << "connect to broker." << std::endl;
    }
    std::vector<std::string> topics;
    consumer->getBrokerTopics(topics);
    if(!topics.empty()){
        for(std::vector<std::string>::iterator iter = topics.begin();
            iter != topics.end(); iter++){
            std::cout << *iter << std::endl;
        }
    }*/
    std::cout << "back to main" << std::endl;
    return 0;
}

