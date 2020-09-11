#ifndef NSEMQ_NSEMQPRODUCER_H_
#define NSEMQ_NSEMQPRODUCER_H_
/**
 * producer/send message header file
 * @author cmy
 */

#ifdef NseMQ_EXPORTS
#define NSE_EXPORT _declspec(dllexport)
#else
#define NSE_EXPORT _declspec(dllimport)
#endif

#include "NseMqBase.h"
#include "NseMqProducerCallback.h"
class NSE_EXPORT NseMqProducer : public NseMqBase{
private:
    std::string broker_addr_;                   // broker address by hostname:port(ie. 127.0.0.1:9092)

    rd_kafka_conf_t *producer_conf_;            // producer configuration
    rd_kafka_t *producer_;                      // producer object pointer
    void *dr_msg_cb_;                           // producer delivery report callback TODO
    int32_t partition_;                         // use default setting:0
public:
    NseMQ::RunStatus run_status_ = NseMQ::NO_INIT;
public:
    NseMqProducer();
    NseMqProducer(std::string broker_addr,
                  void(*dr_msg_cb)(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque));
    ~NseMqProducer();

    // initialize producer and producer configuration.
    NseMQ::ErrorCode init(std::string broker_addr,
                          void(*dr_msg_cb)(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque));

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
    const std::string &getBrokerAddr() const;

    void setBrokerAddr(const std::string &brokerAddr);

    rd_kafka_conf_t *getProducerConf() const;

    void setProducerConf(rd_kafka_conf_t *producerConf);

    rd_kafka_t *getProducer() const;

    void setProducer(rd_kafka_t *producer);

    void *getDrMsgCb() const;

    void setDrMsgCb(void *drMsgCb);

    int32_t getPartition() const;

    void setPartition(int32_t partition);

    NseMQ::RunStatus getRunStatus() const;

    void setRunStatus(NseMQ::RunStatus runStatus);

};

#endif // NSEMQ_NSEMQPRODUCER_H_