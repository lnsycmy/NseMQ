#include "NseMqBase.h"

NseMqBase::NseMqBase(){
    //std::cout << "NseMqBase()" << std::endl;
}

NseMqBase::~NseMqBase(){
    // std::cout << "NseMqBase" << std::endl;
}

bool NseMqBase::judgeConnectionImpl(rd_kafka_t *handle){
    const struct rd_kafka_metadata *metadata;
    rd_kafka_resp_err_t err = rd_kafka_metadata(handle, 1, NULL, &metadata, 1000);
    return (err ==  RD_KAFKA_RESP_ERR_NO_ERROR)?TRUE:FALSE;
}

void NseMqBase::getBrokerTopicsImpl(std::vector<std::string> &topics, rd_kafka_t *handle){
    const struct rd_kafka_metadata *metadata;
    rd_kafka_resp_err_t err = rd_kafka_metadata(handle, 1, NULL, &metadata, 1000);
    if(err != RD_KAFKA_RESP_ERR_NO_ERROR) {
        this->writeErrorLogImpl("Failed to connect to broker.", "NseMQBase");
        this->writeErrorLogImpl(rd_kafka_err2str(err), "NseMQBase");
    }
    // clear the topics.
    topics.clear();
    // TODO:iterator metadata topics.
    int topic_cnt = metadata->topic_cnt;
    rd_kafka_metadata_topic * all_topics = metadata->topics;
    for(int i=0; i < topic_cnt; i++) {
        topics.push_back(all_topics[i].topic);
    }
}

void NseMqBase::writeErrorLogImpl(std::string err_str, std::string write_object){
    if(write_object.empty()){
        std::cerr << "% [NseMQ]" << err_str << std::endl;
    }else{
        std::cerr << "% [NseMQ][" << write_object << "] " << err_str << std::endl;
    }
}