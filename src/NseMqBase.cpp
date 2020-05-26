/**
 * handle implement.
 * @author cmy
 * @date 2020/4/26.
 */
#include "NseMqBase.h"

NseMqBase::NseMqBase(){
    //std::cout << "NseMqBase()" << std::endl;
}

NseMqBase::~NseMqBase(){
    // std::cout << "NseMqBase" << std::endl;
}

bool NseMqBase::judgeConnectionImpl(RdKafka::Handle *handle){
    RdKafka::Metadata *metadata;
    RdKafka::ErrorCode err = handle->metadata(true, NULL,
                                                           &metadata, 1000);
    return (err == RdKafka::ERR_NO_ERROR)?true:false;
}

void NseMqBase::getBrokerTopicsImpl(std::vector<std::string> &topics, RdKafka::Handle *handle){
    RdKafka::Metadata *metadata;
    RdKafka::ErrorCode err = handle->metadata(true, NULL,
                                                           &metadata, 1000);
    if (err != RdKafka::ERR_NO_ERROR) {
        this->writeErrorLogImpl( "Failed to connect to broker.(" + RdKafka::err2str(err) + ")","");
    }
    // clear the topics.
    topics.clear();
    // iterator metadata topics.
    RdKafka::Metadata::TopicMetadataIterator it;
    for (it = metadata->topics()->begin(); it != metadata->topics()->end(); ++it) {
        topics.push_back((*it)->topic());
    }
}

void NseMqBase::writeErrorLogImpl(std::string err_str, std::string write_object){
    if(write_object.empty()){
        std::cerr << "% [NseMQ]" << err_str << std::endl;
    }else{
        std::cerr << "% [NseMQ][" << write_object << "] " << err_str << std::endl;
    }
}
