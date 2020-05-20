//
// Created by cmy on 2020/5/11.
//
#include <iostream>
#include <fstream>
#include <complex>
#include <string>

#include "avro/Encoder.hh"
#include "avro/Decoder.hh"

#include "avro/ValidSchema.hh"
#include "avro/Compiler.hh"
#include "avro/Specific.hh"
#include "student.hh"
#include "NseMQHandle.h"

/*
int main(){
    NseMQ::student s1;
    s1.name="cmy";
    s1.age=24;
    s1.sex="boy";

    NseMQHandle n1;
    const unsigned char *msg =  n1.encode(s1);

    NseMQHandle n2;
    NseMQ::student s2;
    n2.decode(s2, msg, strlen((char *)msg));
    std::cout << "name:" << s2.name
              << " sex:" << s2.sex
              << " age:" << s2.age << std::endl;
    return 0;
}
*/