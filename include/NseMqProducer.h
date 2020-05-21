#ifndef NSEMQ_NSEMQPRODUCER_H_
#define NSEMQ_NSEMQPRODUCER_H_
/**
 * producer/send message header file
 * @author cmy
 * @date 2020/4/26.
 */
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <typeinfo>

#include "NseMqHandle.h"
#include "NseMqSerializer.h"
#include "librdkafka/rdkafkacpp.h"

#define MESSAGE_MAX_SIZE 102400

class NseMqProducer : public NseMqHandle{
private:
    std::string broker_addr_;                   // broker address by hostname:port(ie. 127.0.0.1:9092)
    static std::string errstr_;                 // error string from function.

    RdKafka::Conf *producer_conf_;              // producer configuration
    RdKafka::Producer *producer_;               // producer object pointer
    RdKafka::DeliveryReportCb *producer_dr_cb_; // producer delivery report callback
    int32_t partition_;                         // use default setting:0
    NseMqSerializer serializer_;                // serializer object

public:
    NseMqProducer();
    NseMqProducer(std::string broker_addr, RdKafka::DeliveryReportCb *producer_cb = nullptr);
    ~NseMqProducer();

    // initialize producer and producer configuration.
    NseMQ::ErrorCode init(std::string broker_addr, RdKafka::DeliveryReportCb *producer_cb = nullptr);

    // produce the object 't' to broker, finished serialization internally.
    template <typename T>
    NseMQ::ErrorCode produce(T &t, std::string topic_name){
        // get the data type.
        std::string t_type = typeid(t).name();
        // serialize the massage, message max size limit MESSAGE_MAX_SIZE.
        char *msg = new char[MESSAGE_MAX_SIZE];
        memset(msg, 0, MESSAGE_MAX_SIZE);
        size_t msg_len = MESSAGE_MAX_SIZE;
        serializer_.encode(t, msg, msg_len);
        // produce message.
        return produce(msg, strlen(msg), topic_name, t_type);
    }

    // produce message to broker.
    NseMQ::ErrorCode produce(char *msg, size_t msg_len,std::string topic_name,
                             std::string msg_type = "std::string");

    // close producer and clear memory.
    NseMQ::ErrorCode close();

    bool judgeConnection();                                     // test connection with broker.
    void getBrokerTopics(std::vector<std::string> &topics);     // get all topics from broker.
    void writeErrorLog(std::string err_str);                    // write error log.

    /******************** getter and setter function ********************/
    RdKafka::Conf *getProducerConf() const;

    const std::string &getBrokerAddr() const;

    void setBrokerAddr(const std::string &brokerAddr);

    static const std::string &getErrstr();

    static void setErrstr(const std::string &errstr);

    void setProducerConf(RdKafka::Conf *producerConf);

    RdKafka::Producer *getProducer() const;

    void setProducer(RdKafka::Producer *producer);

    RdKafka::DeliveryReportCb *getProducerDrCb() const;

    void setProducerDrCb(RdKafka::DeliveryReportCb *producerDrCb);

    int32_t getPartition() const;

    void setPartition(int32_t partition);
};

#endif // NSEMQ_NSEMQPRODUCER_H_