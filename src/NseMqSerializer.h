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
    unsigned char * encode(T &t, size_t &msg_len){
        // initialize the memoryOutputStream/encoder.
        out_ = avro::memoryOutputStream();
        encoder_ = avro::binaryEncoder();
        encoder_->init(*out_);
        avro::encode(*encoder_, t);
        encoder_->flush();                  // before byteCount() MUST called flush().
        msg_len = encoder_->byteCount();    // get the
        unsigned char *msg = new unsigned char[msg_len];
        in_ = avro::memoryInputStream(*out_);
        size_t used_byte = 0, n = 0;
        unsigned char *data;
        while(in_->next((const unsigned char**)(&data), &n)){
            memcpy(msg + used_byte, data, n);
            used_byte += n;
        }
        return msg;
    }
    template <typename T>
    bool decode(T &t, const unsigned char * msg, size_t msg_len){
        in_ = avro::memoryInputStream(msg, msg_len);
        decoder_ = avro::binaryDecoder();
        decoder_->init(*in_);
        avro::decode(*decoder_, t);
        return true;
    }
};

#endif //NSEMQ_NSEMQSERIALIZER_H
