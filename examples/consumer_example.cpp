//
// Created by cmy on 2020/5/1.
//
#pragma once
#include <iostream>
#include <thread>
#include <map>

#include "student.hh"
#include "cpx.hh"
#include "../src/NseMqConsumer.h"
#include "../src/NseMqConsumerCallback.h"

template<class T>
class ConsumerCallback : public NseMqConsumerCallback<T>{
public:
    void consume_callback(NseMQ::student &student) {
        std::cout << "ConsumerCallback student" << std::endl;
        std::cout << " NseMQ::student cb name:" << student.name << std::endl;
        std::cout << " NseMQ::student cb age:" << student.age << std::endl;
        std::cout << " NseMQ::student cb sex:" << student.sex << std::endl;
    };
    void consume_callback(NseMQ::cpx &cpx) {
        std::cout << "ConsumerCallback cpx" << std::endl;
        std::cout << " NseMQ::cpx cb im:" << cpx.im << std::endl;
        std::cout << " NseMQ::cpx cb re:" << cpx.re << std::endl;
    };
    void consume_callback(std::string &s){
        std::cout << s <<std::endl;
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
    consumer.start();
    Sleep(10000);
    consumer.close();
    /*auto threadFunction = [&consumer]() {
        while (1){
            consumer.poll();
            std::cout << "receive poll()" << std::endl;
        }
    };
    std::thread thread(threadFunction);
    thread.join();*/
    std::cout << "back to main()" << std::endl;
    return 0;
}

