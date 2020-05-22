#ifndef NSEMQ_NSEMQCONSUMER_H_
#define NSEMQ_NSEMQCONSUMER_H_
/**
 * consumer header file
 * @author cmy
 * @date 2020/4/26
 */
#include <iostream>
#include <string>
#include <map>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <typeinfo>
#include <math.h>

#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#endif

#include "NseMqHandle.h"
#include "NseMqSerializer.h"
#include "librdkafka/rdkafkacpp.h"

#define THREAD_MAX_NUM 20
#define CALLBACK_TIMEOUT_MS 1000

#ifdef _WIN32
static unsigned int __stdcall ThreadFun(void *consumer_ptr);
#endif

class NseMqThreadData;

class NseMqConsumer : public NseMqHandle{
private:
    std::string broker_addr_;                   // broker address by hostname:port(ie. 127.0.0.1:9092)
    static std::string errstr_;                 // error string from function.

    RdKafka::Conf *consumer_conf_;              // consumer configuration
    RdKafka::Consumer *consumer_;               // consumer object pointer

    int64_t start_offset_;                      /* consume message from some offset, optional value:
                                                 * RdKafka::Topic::OFFSET_BEGINNING,
                                                 * RdKafka::Topic::OFFSET_END,
                                                 * RdKafka::Topic::OFFSET_STORED
                                                 */
    int32_t partition_;                         // use default setting:0

    std::map<std::string, RdKafka::ConsumeCb *> topic_cb_map_; // topic and callback mapping.

    /* private function about thread */
#ifdef _WIN32
#define GET_MUTEX() do{WaitForSingleObject(hMutex, INFINITE);}while(0==1)
#define RELASE_MUTEX() do{ReleaseMutex(hMutex);}while(0==1)
    HANDLE handle[THREAD_MAX_NUM];
    HANDLE hMutex = INVALID_HANDLE_VALUE;
    unsigned uiThreadID[THREAD_MAX_NUM];
    NseMqThreadData *threadData;
    bool pollThreadWin();
#endif
public:
    enum RunStatus{
        INIT_STATUS = 0,
        START_STATUS = 1,
        PAUSE_STATUS = 2,
        CLOSE_STATUS = 3,
    };
    RunStatus run_status_;
public:
    NseMqConsumer();
    NseMqConsumer(std::string broker_addr);
    ~NseMqConsumer();

    // initialize consumer.
    NseMQ::ErrorCode init(std::string broker_addr);

    // subscribe to a topic, and bind a consume callback object to topic.
    NseMQ::ErrorCode subscribe(std::string topic_name,
                               RdKafka::ConsumeCb &consume_cb,
                               int64_t start_offset = RdKafka::Topic::OFFSET_END);
    // unsubscribe topic by topic name and need to change topic map.
    NseMQ::ErrorCode unSubscribe(std::string topic_name);
    // get topics name which had subscribed.
    NseMQ::ErrorCode subscription(std::vector<std::string> &topics);

    NseMQ::ErrorCode start();           // start to consume message from broker.

    NseMQ::ErrorCode pause();           // pause the consumer thread.

    NseMQ::ErrorCode resume();          // resume the consumer thread.

    NseMQ::ErrorCode poll();            // polled to call the topic consume callback.

    NseMQ::ErrorCode close();           // close the consumer.

    bool judgeConnection();                                   // test connection with broker.
    void getBrokerTopics(std::vector<std::string> &topics);   // get all topics from broker.
    void writeErrorLog(std::string err_str);                  // write error log.

    /******************** getter and setter function ********************/
    const std::string &getBrokerAddr() const;

    void setBrokerAddr(const std::string &brokerAddr);

    static const std::string &getErrstr();

    static void setErrstr(const std::string &errstr);

    RdKafka::Conf *getConsumerConf() const;

    void setConsumerConf(RdKafka::Conf *consumerConf);

    RdKafka::Consumer *getConsumer() const;

    void setConsumer(RdKafka::Consumer *consumer);

    int64_t getStartOffset() const;

    void setStartOffset(int64_t startOffset);

    int32_t getPartition() const;

    void setPartition(int32_t partition);

    const std::map<std::string, RdKafka::ConsumeCb *> &getTopicCbMap() const;

    void setTopicCbMap(const std::map<std::string, RdKafka::ConsumeCb *> &topicCbMap);

    RunStatus getRunStatus() const;

    void setRunStatus(RunStatus runStatus);
};

/* thread function used data */
class NseMqThreadData{
public:
    int index;
    NseMqConsumer *consumer;
    std::string topic_name;
    RdKafka::ConsumeCb *consume_cb;
#ifdef _WIN32
    HANDLE *handle;
    unsigned uiThreadID;
#endif
};

#endif //NSEMQ_NSEMQCONSUMER_H_
