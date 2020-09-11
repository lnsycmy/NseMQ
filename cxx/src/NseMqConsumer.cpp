#include "NseMqConsumer.h"

// no-parameter constructor.
NseMqConsumer::NseMqConsumer(){
    // initialize pointers and run status.
    consumer_conf_ = NULL;
    consumer_ = NULL;
}
// Constructor with parameter instead of init()
NseMqConsumer::NseMqConsumer(std::string broker_addr) {
    // initialize pointers and run status.
    consumer_conf_ = NULL;
    consumer_ = NULL;
    this->init(broker_addr);    // initialize consumer.
}

NseMqConsumer::~NseMqConsumer() {
    if(run_status_ != NseMQ::CLOSE_STATUS){
        this->close();
    }
}

/**
 * initialize consumer configuration and create consumer.
 * @param broker_addr set the broker address.
 */
NseMQ::ErrorCode NseMqConsumer::init(std::string broker_addr) {
    // set up the default partition and broker address.
    this->setPartition(0);
    this->setBrokerAddr(broker_addr);
    // judge current status
    if(run_status_ != NseMQ::NO_INIT){
        this->writeErrorLog("Don't initialize consumer multiple times.");
        return NseMQ::ERR_C_INIT_MULTIPLE_INIT;
    }
    // create consumer_conf_.
    rd_kafka_conf_res_t conf_res;
    consumer_conf_ = rd_kafka_conf_new(); // Kafka configuration
    // TODO:produce random group id.
    char * group_id = "1000";
    // set random group id(reference timer).
    conf_res = rd_kafka_conf_set(consumer_conf_,"group.id", group_id, errstr_, sizeof(errstr_));

    // set bootstrap broker address and port.

    // Create consumer using accumulated global configuration.
    if (!(consumer_ = rd_kafka_new(RD_KAFKA_CONSUMER, consumer_conf_,
                                   errstr_, sizeof(errstr_)))) {
        this->writeErrorLog("Failed to create new consumer.");
        this->writeErrorLog(errstr_);
        return NseMQ::ERR_C_CREATE_CONSUMER;
    }

    // judge connection with broker.
    if(!this->judgeConnection()){
        this->writeErrorLog("Failed to connect broker ("+ this->getBrokerAddr() + ")");
        return NseMQ::ERR_FAIL_CONNECT_BROKER;
    }
    this->setRunStatus(NseMQ::INIT_STATUS);
    return NseMQ::ERR_NO_ERROR;
}

/**
 * subscribe to a topic, and bind a consume callback object to topic.
 * @param topic_name    topic name that want to subscribe.
 * @param consume_cb    bind subscribed topic and one comsumer callback.
 * @param start_offset  consume message from which paration, default RdKafka::Topic::OFFSET_END
 */
NseMQ::ErrorCode NseMqConsumer::subscribe(std::string topic_name,
                                          void (*consumer_callback)(rd_kafka_message_t *, void *),
                                          int64_t start_offset){
    // judge the run status.
    if(run_status_ != NseMQ::INIT_STATUS){
        this->writeErrorLog("Failed to ubscribe: only allow to subscribe topic before called start().");
        return NseMQ::ERR_C_RUN_STATUS;
    }
    // create topic.
    rd_kafka_topic_t *topic = rd_kafka_topic_new(consumer_, topic_name.c_str(), NULL);
    if (!topic) {
        this->writeErrorLog("Failed to create topic by topic name (" + topic_name +")");
        return NseMQ::ERR_C_SUBS_CREATE_TOPIC;
    }
    // start consumer for topic+partition at start offset.
    int resp = rd_kafka_consume_start(topic, getPartition(), start_offset);
    if (resp != -1) {
        rd_kafka_resp_err_t err_ = rd_kafka_last_error();
        this->writeErrorLog("Failed to subscribe topic.");
        this->writeErrorLog(rd_kafka_err2str(err_));
        if (err_ == RD_KAFKA_RESP_ERR__INVALID_ARG){
            this->writeErrorLog("Broker based offset storage requires a group.id");
        }
        return NseMQ::ERR_C_SUBS_BROKER_TOPIC;
    }
    // add mutex lock, put topic name and callback into map.
    topic_mutex.lock();
    topic_cb_map_.insert(std::pair<std::string,
                        void (*)(rd_kafka_message_t *, void *)>
                         (topic_name, consumer_callback));
    topic_mutex.unlock();
    if(0 == topic_cb_map_.count(topic_name)){
        this->writeErrorLog("Failed to subscribe topic.");
        return NseMQ::ERR_C_SUBS_LOCAL_TOPIC;
    }
    return NseMQ::ERR_NO_ERROR;
}

/**
 * unsubscribe topic by topic name.
 */
NseMQ::ErrorCode NseMqConsumer::unSubscribe(std::string topic_name){
    // judge the run status.
    if(run_status_ != NseMQ::INIT_STATUS){
        this->writeErrorLog("Failed to unsubscribe: only allow to unsubscribe topic before called start().");
        return NseMQ::ERR_C_RUN_STATUS;
    }
    // verify that the topic callback map is empty or not includes topic_name.
    if(topic_cb_map_.empty() || 0 == topic_cb_map_.count(topic_name)){
        this->writeErrorLog("% topic subscription is empty or havn't subscribed the topic:" + topic_name);
        return NseMQ::ERR_C_UNSUBS_TOPIC_NO_FIND;
    }
    // create topic, call consumer->stop(), cancle subscribe the topic.
    rd_kafka_topic_t *topic = rd_kafka_topic_new(consumer_, topic_name.c_str(), NULL);
    int resp = rd_kafka_consume_stop(topic, getPartition());
    if(resp != 0){
        rd_kafka_resp_err_t err_ = rd_kafka_last_error();
        this->writeErrorLog("% Failed to unsubscribe topic "+ topic_name);
        this->writeErrorLog(rd_kafka_err2str(err_));
        return NseMQ::ERR_C_UNSUNS_BROKER_TOPIC;
    }
    // add mutex lock, delete topic from topic callback map.
    topic_mutex.lock();
    topic_cb_map_.erase(topic_name);
    topic_mutex.unlock();
    return NseMQ::ERR_NO_ERROR;
}

/**
 * get have subscribed topics.
 * @param topics
 */
NseMQ::ErrorCode NseMqConsumer::subscription(std::vector<std::string> &topics){
    if(topic_cb_map_.empty()){
        return NseMQ::ERR_C_SUBS_TOPIC_EMPTY;
    }else{
        for(std::map<std::string, void *>::iterator iter = topic_cb_map_.begin();
            iter != topic_cb_map_.end(); iter++){
            topics.push_back(iter->first);
        }
    }
    return NseMQ::ERR_NO_ERROR;
}

/**
 * polled to call the topic consume callback.
 * if want to no-blocking, user need to defined thread that cyclic called poll().
 */
NseMQ::ErrorCode NseMqConsumer::poll(){
    // judge subscribed topic is not empty.
    if(topic_cb_map_.empty()){
        this->writeErrorLog("Failed to consume: don't have subscribe one topic.");
        return NseMQ::ERR_C_POLL_TOPIC_EMPTY;
    }
    // all topics consume_callback use 1000ms every poll.
    size_t topic_count = topic_cb_map_.size();
    int timeout_ms =  floor(CALLBACK_TIMEOUT_MS/topic_count);
    for(std::map<std::string, void *>::iterator iter = topic_cb_map_.begin();
        iter != topic_cb_map_.end(); iter++){
        rd_kafka_topic_t *topic = rd_kafka_topic_new(consumer_,  iter->first.c_str(), NULL);
        rd_kafka_consume_callback(topic,
                                  getPartition(),
                                  1000,/* timeout_ms */
                                   /* consumer callback function */
                                  (void (*)(rd_kafka_message_t *, void *))iter->second,
                                  NULL/*	void* opaque */);
        rd_kafka_poll(consumer_, 0);
    }
    return NseMQ::ERR_NO_ERROR;
}

/**
 * start to consume message from broker, include multiple threads.
 * one topic corresponds to one thread that cyclic called poll().
 */
NseMQ::ErrorCode NseMqConsumer::start(){
    // judge the run status.
    if(run_status_ >= NseMQ::START_STATUS){
        this->writeErrorLog("Failed to start: can't multiple called start() function.");
        return NseMQ::ERR_C_RUN_STATUS;
    }
    // judge subscribed topic is not empty.
    if(topic_cb_map_.empty()){
        this->writeErrorLog("Failed to consume: don't have subscribe one topic.");
        return NseMQ::ERR_C_POLL_TOPIC_EMPTY;
    }
    // start the poll thread.
    for(std::map<std::string, void *>::iterator iter = topic_cb_map_.begin();
        iter != topic_cb_map_.end(); iter++){
        thread_group_.create_thread(boost::bind(&NseMqConsumer::pollThreadFunction,this,
                                                iter->first, iter->second));
    }
    this->setRunStatus(NseMQ::START_STATUS);
    return NseMQ::ERR_NO_ERROR;
}

void NseMqConsumer::pollThreadFunction(std::string topic_name, void *consume_cb) {
    std::string err_str;
    rd_kafka_topic_t *topic = rd_kafka_topic_new(consumer_, topic_name.c_str(), NULL);
    try{
        while(1){
            boost::this_thread::interruption_point();
            rd_kafka_consume_callback(topic,
                                      getPartition(),
                                      CALLBACK_TIMEOUT_MS,/* timeout_ms */
                                      /* consumer callback function */
                                      (void (*)(rd_kafka_message_t *, void *))consume_cb,
                                      NULL/*	void* opaque */);
            rd_kafka_consumer_poll(consumer_, 0);
            boost::mutex::scoped_lock lock(io_mutex);
            std::cout << "% [NseMQ] receive poll()" << std::endl;
        }
    }catch(boost::thread_interrupted& ){
        // thread interrupted, no operation.
    }
}

/* close consumer and clear memory. */
NseMQ::ErrorCode NseMqConsumer::close(){
    // judge the run status.
    if(run_status_ == NseMQ::CLOSE_STATUS){
        this->writeErrorLog("Failed to close: can't multiple called close() function.");
        return NseMQ::ERR_C_RUN_STATUS;
    }
    // NO.1 stop the consume from broker.
    if(!topic_cb_map_.empty()){
        for(std::map<std::string, void *>::iterator iter = topic_cb_map_.begin();
            iter != topic_cb_map_.end(); iter++){
            // create topic, call consumer->stop(), cancle subscribe the topic.
            rd_kafka_topic_t *topic = rd_kafka_topic_new(consumer_, iter->first.c_str(), NULL);
            int resp = rd_kafka_consume_stop(topic, getPartition());
            if(resp != 0){
                rd_kafka_resp_err_t err_ = rd_kafka_last_error();
                this->writeErrorLog("% Failed to stop consuming topic "+ iter->first);
                this->writeErrorLog(rd_kafka_err2str(err_));
                return NseMQ::ERR_C_UNSUNS_BROKER_TOPIC;
            }
        }
        topic_cb_map_.clear();
    }
    // NO.2 end the consumer thread.
    if(thread_group_.size() > 0){
        thread_group_.interrupt_all();
        thread_group_.join_all();
    }
    // NO.3 delete other object.
    delete consumer_conf_;
    // NO.4 delete consumer object.
    rd_kafka_destroy(consumer_);
    // NO.5 wait destroy consumer.
    rd_kafka_wait_destroyed(5000);
    this->setRunStatus(NseMQ::CLOSE_STATUS);
    return NseMQ::ERR_NO_ERROR;
}

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

rd_kafka_conf_t *NseMqConsumer::getConsumerConf() const {
    return consumer_conf_;
}

void NseMqConsumer::setConsumerConf(rd_kafka_conf_t *consumerConf) {
    consumer_conf_ = consumerConf;
}

rd_kafka_t *NseMqConsumer::getConsumer() const {
    return consumer_;
}

void NseMqConsumer::setConsumer(rd_kafka_t *consumer) {
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

const std::map<std::string, void *> &NseMqConsumer::getTopicCbMap() const {
    return topic_cb_map_;
}

void NseMqConsumer::setTopicCbMap(const std::map<std::string, void *> &topicCbMap) {
    topic_cb_map_ = topicCbMap;
}

NseMQ::RunStatus NseMqConsumer::getRunStatus() const {
    return run_status_;
}

void NseMqConsumer::setRunStatus(NseMQ::RunStatus runStatus) {
    run_status_ = runStatus;
}





