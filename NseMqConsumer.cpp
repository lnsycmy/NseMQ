#include "NseMqConsumer.h"

// initialize the static variable errstr_.
std::string NseMqConsumer::errstr_ = "";

// no-parameter constructor.
NseMqConsumer::NseMqConsumer(){
    // initialize pointers
    consumer_conf_ = NULL;
    consumer_ = NULL;
}

NseMqConsumer::NseMqConsumer(std::string broker_addr) {
    // initialize pointers
    consumer_conf_ = NULL;
    consumer_ = NULL;
    this->init(broker_addr);    // initialize consumer.
}

NseMqConsumer::~NseMqConsumer() {
    this->close();
}

NseMQ::ErrorCode NseMqConsumer::init(std::string broker_addr) {
    // set up the default partition and broker address.
    this->setPartition(0);
    this->setBrokerAddr(broker_addr);
    // create consumer_conf_.
    this->setConsumerConf(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
    // set bootstrap broker address and port.
    if (getConsumerConf()->set("bootstrap.servers", getBrokerAddr(), errstr_) !=
        RdKafka::Conf::CONF_OK) {
        this->writeErrorLog(errstr_);
        return NseMQ::ERR_CONF_BROKER_ADDR;
    }
    // Create consumer using accumulated global configuration.
    this->setConsumer(RdKafka::Consumer::create(getConsumerConf(), errstr_));
    if(!getConsumer()){
        this->writeErrorLog(errstr_);
        return NseMQ::ERR_CREATE_CONSUMER;
    }
    // create mutex lock
    hMutex = CreateMutex(NULL, FALSE, NULL);
    return NseMQ::ERR_NO_ERROR;
}

/**
 * subscribe to a topic, and bind a consume callback object to topic.
 * @param topic_name
 * @param consume_cb
 * @param start_offset
 * @return
 */
NseMQ::ErrorCode NseMqConsumer::subscribe(std::string topic_name,
                                          RdKafka::ConsumeCb &consume_cb,
                                          int64_t start_offset){
    // judge connection with broker.
    if(!this->judgeConnection()){
        this->writeErrorLog("Failed to connect broker ("+ this->getBrokerAddr() + ")");
        return NseMQ::ERR_FAIL_CONNECT_BROKER;
    }
    // create topic.
    RdKafka::Topic *topic = RdKafka::Topic::create(getConsumer(), topic_name, NULL, errstr_);
    if (!topic) {
        this->writeErrorLog("Failed to create topic by topic name (" + topic_name +")");
        return NseMQ::ERR_SUBS_FAIL_CREATE_TOPIC;
    }
    // start consumer for topic+partition at start offset.
    RdKafka::ErrorCode resp = consumer_->start(topic, getPartition(), start_offset);
    if (resp != RdKafka::ERR_NO_ERROR) {
        std::cerr << "resp:" << resp << std::endl;
        this->writeErrorLog("Failed to subscribe topic (" + RdKafka::err2str(resp) + ")");
        return NseMQ::ERR_SUBS_FAIL_START_CONSUMER;
    }
    // add mutex lock, put topic name and callback into map.
    GET_MUTEX();
    topic_cb_map_.insert(std::pair<std::string, RdKafka::ConsumeCb *>(topic_name, &consume_cb));
    RELASE_MUTEX();
    if(0 == topic_cb_map_.count(topic_name)){
        this->writeErrorLog("Failed to subscribe topic (" + RdKafka::err2str(resp) + ")");
        return NseMQ::ERR_SUBS_FAIL_BIND_CALLBACK;
    }
    return NseMQ::ERR_NO_ERROR;
}

NseMQ::ErrorCode NseMqConsumer::unSubscribe(std::string topic_name){
    // verify that the topic callback map is empty or not includes topic_name.
    if(topic_cb_map_.empty() || 0 == topic_cb_map_.count(topic_name)){
        this->writeErrorLog("% topic subscription is empty or havn't subscribed the topic:" + topic_name);
        return NseMQ::ERR_UNSUBS_TOPIC_NO_FIND;
    }
    // create topic, call consumer->stop(), cancle subscribe the topic.
    RdKafka::Topic *topic = RdKafka::Topic::create(getConsumer(), topic_name, NULL, errstr_);
    RdKafka::ErrorCode resp = consumer_->stop(topic,getPartition());
    if(resp != RdKafka::ERR_NO_ERROR){
        this->writeErrorLog("% Failed to unsubscribe topic "+ topic_name +": " + RdKafka::err2str(resp));
        return NseMQ::ERR_UNSUNS_TOPIC_FAILED;
    }
    // add mutex lock, delete topic from topic callback map.
    GET_MUTEX();
    topic_cb_map_.erase(topic_name);
    RELASE_MUTEX();
    return NseMQ::ERR_NO_ERROR;
}

NseMQ::ErrorCode NseMqConsumer::subscription(std::vector<std::string> &topics){
    if(topic_cb_map_.empty()){
        return NseMQ::ERR_SUBS_TOPIC_EMPTY;
    }else{
        for(std::map<std::string, RdKafka::ConsumeCb *>::iterator iter = topic_cb_map_.begin();
            iter != topic_cb_map_.end(); iter++){
            topics.push_back(iter->first);
        }
    }
    return NseMQ::ERR_NO_ERROR;
}

NseMQ::ErrorCode NseMqConsumer::poll(){
    // judge subscribed topic is not empty.
    if(topic_cb_map_.empty()){
        this->writeErrorLog("Failed to consume: don't have subscribe one topic.");
        return NseMQ::ERR_START_TOPIC_EMPTY;
    }
    // all topics consume_callback use 1000ms every poll.
    size_t topic_count = topic_cb_map_.size();
    int timeout_ms =  floor(1000/topic_count);
    for(std::map<std::string,RdKafka::ConsumeCb *>::iterator iter = topic_cb_map_.begin();
        iter != topic_cb_map_.end(); iter++){
        RdKafka::Topic *topic = RdKafka::Topic::create(getConsumer(), iter->first, NULL, errstr_);
        consumer_->consume_callback(topic, getPartition(), timeout_ms, iter->second, NULL);
    }
    consumer_->poll(0);
    return NseMQ::ERR_NO_ERROR;
}

NseMQ::ErrorCode NseMqConsumer::start(){
    // judge subscribed topic is not empty.
    if(topic_cb_map_.empty()){
        this->writeErrorLog("Failed to consume: don't have subscribe one topic.");
        return NseMQ::ERR_START_TOPIC_EMPTY;
    }
    // start the thread with different platforms.
#ifdef _WIN32
    if(!this->pollThreadWin()){
        return NseMQ::ERR_START_CREATE_THREAD;
    }
#endif
    return NseMQ::ERR_NO_ERROR;
}

NseMQ::ErrorCode NseMqConsumer::close(){
    // NO.1 stop the consume from broker.
    if(!topic_cb_map_.empty()){
        for(std::map<std::string,RdKafka::ConsumeCb *>::iterator iter = topic_cb_map_.begin();
            iter != topic_cb_map_.end(); iter++){
            this->unSubscribe(iter->first);
        }
    }
    // TODO:NO.2 flush the local queue.

    // NO.3 end the consumer thread.
#ifdef _WIN32
    for(int i = 0; i < THREAD_MAX_NUM; i++){
        if(uiThread1ID[i] != NULL){
            _endthreadex(uiThread1ID[i]);
        }
    }
#endif
    // NO.4 delete other object.
    delete consumer_conf_;
    if(!topic_cb_map_.empty()){
        topic_cb_map_.clear();
    }
    // NO.5 delete consumer object.
    delete consumer_;
    // NO.6 wait for RdKafka to decommission.
    RdKafka::wait_destroyed(2000);
    return NseMQ::ERR_NO_ERROR;
}

NseMQ::ErrorCode NseMqConsumer::pause(){
    // pause the consume thread.
#ifdef _WIN32
    for(int i = 0; i < THREAD_MAX_NUM; i++){
        if(handle[i] != NULL){
            SuspendThread(handle[i]);
        }
    }
#endif
    return NseMQ::ERR_NO_ERROR;
}

NseMQ::ErrorCode NseMqConsumer::resume(){
#ifdef _WIN32
    for(int i = 0; i < THREAD_MAX_NUM; i++){
        if(handle[i] != NULL){
            ResumeThread(handle[i]);
        }
    }
#endif
    return NseMQ::ERR_NO_ERROR;
}

#ifdef _WIN32
bool NseMqConsumer::pollThreadWin(){
    threadData = new NseMqThreadData[topic_cb_map_.size()];
    int i = 0;
    for(std::map<std::string,RdKafka::ConsumeCb *>::iterator iter = topic_cb_map_.begin();
        iter != topic_cb_map_.end(); iter++){
        threadData[i].index = i;
        threadData[i].consumer = this;
        threadData[i].topic_name = iter->first;
        threadData[i].consume_cb = iter->second;
        handle[i] = (HANDLE)_beginthreadex(NULL, 0,ThreadFun, &threadData[i],
                                           0, &uiThread1ID[i]);
        if(handle[i] == NULL){
            return false;
        }else{
            threadData[i].handle = &handle[i];
        }
        ++i;
    }
    // WaitForMultipleObjects(i, handle, TRUE, INFINITE);
    // std::cout << "after WaitForMultipleObjects()" << std::endl;
    return true;
}

static unsigned int __stdcall ThreadFun(void *threadParam){
    NseMqThreadData *threadData = (NseMqThreadData *)threadParam;
    NseMqConsumer *consumer = (NseMqConsumer *)(threadData->consumer);
    std::string err_str;
    while(1){
        RdKafka::Topic *topic = RdKafka::Topic::create(consumer->getConsumer(), threadData->topic_name, NULL, err_str);
        consumer->getConsumer()->consume_callback(topic, consumer->getPartition(),
                                                  CALLBACK_TIMEOUT_MS, threadData->consume_cb, NULL);
        consumer->getConsumer()->poll(0);
        std::cout << "% [NseMQ] receive poll()" << std::endl;
    }
    return 0;
}
#endif

bool NseMqConsumer::judgeConnection() {
    return this->judgeConnectionImpl(this->getConsumer());
}

void NseMqConsumer::getBrokerTopics(std::vector<std::string> &topics){
    this->getBrokerTopicsImpl(topics,this->getConsumer());
}

void NseMqConsumer::writeErrorLog(std::string err_str){
    this->writeErrorLogImpl(err_str, "Consumer");
}

/******************** getter and setter implement function ********************/
const std::string &NseMqConsumer::getBrokerAddr() const {
    return broker_addr_;
}

void NseMqConsumer::setBrokerAddr(const std::string &brokerAddr) {
    broker_addr_ = brokerAddr;
}
const std::string &NseMqConsumer::getErrstr() {
    return errstr_;
}

void NseMqConsumer::setErrstr(const std::string &errstr) {
    errstr_ = errstr;
}

RdKafka::Conf *NseMqConsumer::getConsumerConf() const {
    return consumer_conf_;
}

void NseMqConsumer::setConsumerConf(RdKafka::Conf *consumerConf) {
    consumer_conf_ = consumerConf;
}

RdKafka::Consumer *NseMqConsumer::getConsumer() const {
    return consumer_;
}

void NseMqConsumer::setConsumer(RdKafka::Consumer *consumer) {
    consumer_ = consumer;
}

int64_t NseMqConsumer::getStartOffset() const {
    return start_offset_;
}

void NseMqConsumer::setStartOffset(int64_t startOffset) {
    start_offset_ = startOffset;
}

int32_t NseMqConsumer::getPartition() const {
    return partition_;
}

void NseMqConsumer::setPartition(int32_t partition) {
    partition_ = partition;
}

const std::map<std::string, RdKafka::ConsumeCb *> &NseMqConsumer::getTopicCbMap() const {
    return topic_cb_map_;
}

void NseMqConsumer::setTopicCbMap(const std::map<std::string, RdKafka::ConsumeCb *> &topicCbMap) {
    topic_cb_map_ = topicCbMap;
}


