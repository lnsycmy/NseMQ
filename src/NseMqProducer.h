#ifndef NSEMQ_NSEMQPRODUCER_H_
#define NSEMQ_NSEMQPRODUCER_H_
/**
 * producer/send message header file
 * @author cmy
 * @date 2020/4/26.
 */
#ifdef NseMQ_EXPORTS
#define NSE_EXPORT _declspec(dllexport)
#else
#define NSE_EXPORT _declspec(dllimport)
#endif

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <typeinfo>
#include <librdkafka/rdkafkacpp.h>
#include "NseMqBase.h"

class NSE_EXPORT NseMqProducer : public NseMqBase{
private:
    std::string broker_addr_;                   // broker address by hostname:port(ie. 127.0.0.1:9092)

    RdKafka::Conf *producer_conf_;              // producer configuration
    RdKafka::Producer *producer_;               // producer object pointer
    RdKafka::DeliveryReportCb *producer_dr_cb_; // producer delivery report callback
    int32_t partition_;                         // use default setting:0
public:
    enum RunStatus{
        INIT_STATUS = 0,
        START_STATUS = 1,
        CLOSE_STATUS = 2,
    };
    RunStatus run_status_;
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
        // serialize the massage
        size_t msg_len = 0;
        unsigned char * msg = this->encode(t, msg_len);
        // produce message.
        return produce((char *)msg, msg_len, topic_name, t_type);
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

    RunStatus getRunStatus() const;

    void setRunStatus(RunStatus runStatus);
};

#endif // NSEMQ_NSEMQPRODUCER_H_