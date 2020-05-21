/**
 * serialized data header file
 * @author cmy
 * @date 2020/4/26.
 */
#ifndef NSEMQ_NSEMQSERIALIZER_H
#define NSEMQ_NSEMQSERIALIZER_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"
#include "avro/ValidSchema.hh"
#include "avro/Compiler.hh"
#include "avro/Specific.hh"

class NseMqSerializer {
private:
    std::unique_ptr<avro::OutputStream> out_;
    std::unique_ptr<avro::InputStream> in_;
    avro::EncoderPtr encoder_;
    avro::DecoderPtr decoder_;
public:
    ~NseMqSerializer(){};
    template <typename T>
    void encode(T &t, char *msg, size_t &msg_len){
        // initialize the memoryOutputStream/encoder.
        out_ = avro::memoryOutputStream();
        encoder_ = avro::binaryEncoder();
        encoder_->init(*out_);
        // TODO:deal with empty value.
        avro::encode(*encoder_, t);
        in_ = avro::memoryInputStream(*out_);
        size_t total = 0, n = 0;
        const unsigned char *temp = NULL;
        while(in_->next(&temp, &n)){
            total += n;
            strcat_s(msg, msg_len,(const char *)(char *)temp);
        }
        msg_len = total;
    }
    template <typename T>
    bool decode(T &t, const unsigned char * msg){
        in_ = avro::memoryInputStream(msg, strlen((char *)msg));
        decoder_ = avro::binaryDecoder();
        decoder_->init(*in_);
        avro::decode(*decoder_, t);
        return true;
    }
};

#endif //NSEMQ_NSEMQSERIALIZER_H
