/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef STUDENT_HH_3269787986__H_
#define STUDENT_HH_3269787986__H_


#include <sstream>
#include "boost/any.hpp"
#include "avro/Specific.hh"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"

namespace NseMQ {
struct student {
    std::string name;
    int32_t age;
    std::string sex;
    student() :
        name(std::string()),
        age(int32_t()),
        sex(std::string())
        { }
};

}
namespace avro {
template<> struct codec_traits<NseMQ::student> {
    static void encode(Encoder& e, const NseMQ::student& v) {
        avro::encode(e, v.name);
        avro::encode(e, v.age);
        avro::encode(e, v.sex);
    }
    static void decode(Decoder& d, NseMQ::student& v) {
        if (avro::ResolvingDecoder *rd =
            dynamic_cast<avro::ResolvingDecoder *>(&d)) {
            const std::vector<size_t> fo = rd->fieldOrder();
            for (std::vector<size_t>::const_iterator it = fo.begin();
                it != fo.end(); ++it) {
                switch (*it) {
                case 0:
                    avro::decode(d, v.name);
                    break;
                case 1:
                    avro::decode(d, v.age);
                    break;
                case 2:
                    avro::decode(d, v.sex);
                    break;
                default:
                    break;
                }
            }
        } else {
            avro::decode(d, v.name);
            avro::decode(d, v.age);
            avro::decode(d, v.sex);
        }
    }
};

}
#endif
