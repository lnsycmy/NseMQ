#ifndef NSEMQ_NSEMQCONSUMER_H_
#define NSEMQ_NSEMQCONSUMER_H_
/**
 * consumer header file
 * @author cmy
 */
#ifdef NseMQ_EXPORTS
#define NSE_EXPORT _declspec(dllexport)
#else
#define NSE_EXPORT  _declspec(dllimport)
#endif

#define BOOST_ALL_DYN_LINK
#include <map>
#include <math.h>
#include <time.h>
#include <boost/thread.hpp>
#include "NseMqBase.h"
#include "NseMqConsumerCallback.h"

#define CALLBACK_TIMEOUT_MS 1000

// NSE_EXPORT
class NSE_EXPORT NseMqConsumer : public NseMqBase{
private:
    std::string broker_addr_;                   // broker address by hostname:port(ie. 127.0.0.1:9092)

    rd_kafka_conf_t *consumer_conf_;              // consumer configuration
    rd_kafka_t *consumer_;               // consumer object pointer

    int64_t start_offset_;                      /* consume message from some offset, optional value:
                                                 * RdKafka::Topic::OFFSET_BEGINNING,
                                                 * RdKafka::Topic::OFFSET_END,

                                                 * RdKafka::Topic::OFFSET_STORED
                                                 */
    int32_t partition_;                         // use default setting:0

    // 目前传递函数地址，kafka调用回调函数后，再调用对应参数类型的函数；
    std::map<std::string, void *> topic_cb_map_; // topic and callback mapping.

    boost::thread_group thread_group_;          // consumer thread group.
    /* private function about thread */
    void pollThreadFunction(std::string topic_name,
                            void *consume_cb);    // start thread function.
public:
    NseMQ::RunStatus run_status_ = NseMQ::NO_INIT; // consumer class current status.
    boost::mutex topic_mutex;                   // topic mutex.
    boost::mutex io_mutex;                      // iostream mutex.
public:
    NseMqConsumer();
    NseMqConsumer(std::string broker_addr);
    ~NseMqConsumer();

    // initialize consumer.
    NseMQ::ErrorCode init(std::string broker_addr);

    // subscribe to a topic, and bind a consume callback object to topic.
    NseMQ::ErrorCode subscribe(std::string topic_name,
                               void *consume_cb,
                               int64_t start_offset = RD_KAFKA_OFFSET_END);
    // unsubscribe topic by topic name and need to change topic map.
    NseMQ::ErrorCode unSubscribe(std::string topic_name);
    // get topics name which had subscribed.
    NseMQ::ErrorCode subscription(std::vector<std::string> &topics);

    NseMQ::ErrorCode start();           // start to consume message from broker.

    NseMQ::ErrorCode poll();            // polled to call the topic consume callback.

    NseMQ::ErrorCode close();           // close the consumer.

    bool judgeConnection();                                   // test connection with broker.
    void getBrokerTopics(std::vector<std::string> &topics);   // get all topics from broker.
    void writeErrorLog(std::string err_str);                  // write error log.

    /******************** getter and setter function ********************/
    const std::string &getBrokerAddr() const;

    void setBrokerAddr(const std::string &brokerAddr);

    rd_kafka_conf_t *getConsumerConf() const;

    void setConsumerConf(rd_kafka_conf_t *consumerConf);

    rd_kafka_t *getConsumer() const;

    void setConsumer(rd_kafka_t *consumer);

    int64_t getStartOffset() const;

    void setStartOffset(int64_t startOffset);

    int32_t getPartition() const;

    void setPartition(int32_t partition);

    NseMQ::RunStatus getRunStatus() const;

    void setRunStatus(NseMQ::RunStatus runStatus);

    const std::map<std::string, void *> &getTopicCbMap() const;

    void setTopicCbMap(const std::map<std::string, void *> &topicCbMap);
};

#endif //NSEMQ_NSEMQCONSUMER_H_
