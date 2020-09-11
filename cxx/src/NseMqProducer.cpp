#include  "NseMqProducer.h"
NseMqProducer::NseMqProducer() {
    // initialize pointers
    producer_conf_ = NULL;
    producer_ = NULL;
    dr_msg_cb_ = NULL;
}

/**
 * Constructor with parameters instead of init ()
 * @param broker_addr
 * @param producer_cb
 */
NseMqProducer::NseMqProducer(std::string broker_addr,
                             void(*dr_msg_cb)(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque)) {
    // initialize pointers
    producer_conf_ = NULL;
    producer_ = NULL;
    dr_msg_cb_ = NULL;
    this->init(broker_addr, dr_msg_cb);
}

NseMqProducer::~NseMqProducer() {
    this->close();
}

/**
 * initialize producer configuration and create producer.
 * @param broker_addr set the broker address.
 * @param producer_cb set the producer callback.
 * @return NseMQ::ErrorCod suggest reference NseMqBase.h
 */
NseMQ::ErrorCode NseMqProducer::init(std::string broker_addr,
                                     void(*dr_msg_cb)(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque)) {
    // set the broker address/partition/producer delievry callback
    this->setBrokerAddr(broker_addr);
    this->setPartition(RD_KAFKA_PARTITION_UA);
    // create producer_conf_.
    producer_conf_ = rd_kafka_conf_new();

    // Set bootstrap broker(s) as a comma-separated list of host or host:port (default port 9092).
    if (rd_kafka_conf_set(producer_conf_, "bootstrap.servers", broker_addr.c_str(),
                          errstr_, sizeof(errstr_)) != RD_KAFKA_CONF_OK) {
        this->writeErrorLog(errstr_);
        return NseMQ::ERR_P_INIT_BROKER_ADDRESS;
    }

    // set the producer delivery report callback
    if (NULL != dr_msg_cb) {
        rd_kafka_conf_set_dr_msg_cb(producer_conf_, dr_msg_cb);
    }

    // create producer instance using accumulated global configuration.
    producer_ = rd_kafka_new(RD_KAFKA_PRODUCER, producer_conf_, errstr_, sizeof(errstr_));
    if (!producer_) {
        std::string errtemp(errstr_);
        this->writeErrorLog("Failed to create new producer." + errtemp);
        return NseMQ::ERR_P_CREATE_PRODUCER;
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
 * internal use produce message, called by 'produce(T &t, std::string topic_name)'
 * @param msg: message that wait to produce
 * @param msg_len: actual message length
 * @param topic_name: topic that wait to send
 * @param msg_type: message type, e.g. student, defalut std::string
 */
NseMQ::ErrorCode NseMqProducer::produce(char *msg, size_t msg_len, std::string topic_name,
                                        std::string msg_type /*default NULL*/) {
    rd_kafka_resp_err_t err;
    BOOL resend_flag = FALSE;       // control queue full resend.
    NseMQ::ErrorCode err_temp = NseMQ::ERR_NO_ERROR;
    size_t len = strlen(msg);
    // judge the run status.
    if(run_status_ == NseMQ::CLOSE_STATUS){
        this->writeErrorLog("Failed to produce: don't allow call produce() after close().");
        return NseMQ::ERR_P_RUN_STATUS;
    }else if(run_status_ != NseMQ::START_STATUS){
        this->setRunStatus(NseMQ::START_STATUS);
    }
    // judge message whether empty.
    if(0 == msg_len){
        rd_kafka_poll(producer_, 0/*non-blocking */);
        return NseMQ::ERR_P_SEND_MSG_EMPTY;
    }

    // Produce message.
    retry:
    err = rd_kafka_producev(
            /* Producer handle */
            producer_,
            /* Topic name */
            RD_KAFKA_V_TOPIC(topic_name.c_str()),
            /* Make a copy of the payload. */
            RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
            /* Message value and length */
            RD_KAFKA_V_VALUE(msg, len),
            /* Message key pointer and length (const void *, size_t) */
            RD_KAFKA_V_KEY(msg_type.c_str(),sizeof(msg_type)),
            /* Per-Message opaque */
            RD_KAFKA_V_OPAQUE(NULL),
            /* End sentinel */
            RD_KAFKA_V_END);

    if (err != RD_KAFKA_RESP_ERR_NO_ERROR) {
        this->writeErrorLog("% Failed to produce to topic(" + topic_name + "):"
                            + rd_kafka_err2str(err));
        if (err == RD_KAFKA_RESP_ERR__QUEUE_FULL) {
            rd_kafka_poll(producer_, 1000/*block for max 1000ms*/);
            if (!resend_flag) { // allow once resend.
                resend_flag = true;
                goto retry;
            } else {
                err_temp =  NseMQ::ERR_P_SEND_QUEUE_FULL;
            }
        } else if (err == RD_KAFKA_RESP_ERR_MSG_SIZE_TOO_LARGE) {

            this->writeErrorLog("% message size it too large.");
            err_temp = NseMQ::ERR_P_SEND_MSG_TOO_LARGE;
        } else if (err == RD_KAFKA_RESP_ERR__UNKNOWN_TOPIC) {
            this->writeErrorLog("% broker don't have the topic [" + topic_name + "]");
            err_temp = NseMQ::ERR_P_SEND_UNKNOWN_TOPIC;
        } else {
            err_temp = NseMQ::ERR_P_SEND_FAIL;
        }
    } else {
        std::cout << "% Enqueued message (" << strlen(msg) << " bytes) " <<
                  "for topic " << topic_name << std::endl;
        err_temp = NseMQ::ERR_NO_ERROR;
    }
    rd_kafka_poll(producer_, 1000);
    delete msg;             // release the message memory
    return err_temp;
}

/**
 * close producer and clear memory.
 */
NseMQ::ErrorCode NseMqProducer::close(){
    // judge the run status.
    if(run_status_ == NseMQ::CLOSE_STATUS){
        this->writeErrorLog("Failed to close: can't multiple called close() function.");
        return NseMQ::ERR_P_RUN_STATUS;
    }
    // flush message: make sure all outstanding requests are transmitted and handled
    rd_kafka_flush(producer_, 5 * 1000 /* wait for max 5 seconds */);
    while (rd_kafka_outq_len(producer_) > 0) {
        this->writeErrorLog("Waiting for message " + rd_kafka_outq_len(producer_));
        rd_kafka_poll(producer_, 1000);
    }

    // delete other object.
    if(NULL != producer_conf_){
		// rd_kafka_conf_destroy(producer_conf_);
    }

    // delete producer object.
    if(NULL != producer_){
        // rd_kafka_destroy(producer_);
    }

    // wait for RdKafka to decommission.
    rd_kafka_wait_destroyed(5000);
    this->setRunStatus(NseMQ::CLOSE_STATUS);
    return NseMQ::ERR_NO_ERROR;
}

bool NseMqProducer::judgeConnection() {
    return this->judgeConnectionImpl(this->getProducer());
}

void NseMqProducer::getBrokerTopics(std::vector<std::string> &topics) {
    this->getBrokerTopicsImpl(topics, this->getProducer());
}
void NseMqProducer::writeErrorLog(std::string err_str){
    this->writeErrorLogImpl(err_str, "Producer");
}



/******************** getter and setter implement function ********************/

const std::string &NseMqProducer::getBrokerAddr() const {
    return broker_addr_;
}

void NseMqProducer::setBrokerAddr(const std::string &brokerAddr) {
    broker_addr_ = brokerAddr;
}

rd_kafka_conf_t *NseMqProducer::getProducerConf() const {
    return producer_conf_;
}

void NseMqProducer::setProducerConf(rd_kafka_conf_t *producerConf) {
    producer_conf_ = producerConf;
}

rd_kafka_t *NseMqProducer::getProducer() const {
    return producer_;
}

void NseMqProducer::setProducer(rd_kafka_t *producer) {
    producer_ = producer;
}

void *NseMqProducer::getDrMsgCb() const {
    return dr_msg_cb_;
}

void NseMqProducer::setDrMsgCb(void *drMsgCb) {
    dr_msg_cb_ = drMsgCb;
}

int32_t NseMqProducer::getPartition() const {
    return partition_;
}

void NseMqProducer::setPartition(int32_t partition) {
    partition_ = partition;
}

NseMQ::RunStatus NseMqProducer::getRunStatus() const {
    return run_status_;
}

void NseMqProducer::setRunStatus(NseMQ::RunStatus runStatus) {
    run_status_ = runStatus;
}





