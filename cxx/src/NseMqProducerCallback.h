#ifndef NSEMQ_CXX_PRODUCERCALLBACK_H
#define NSEMQ_CXX_PRODUCERCALLBACK_H

#include "NseMqBase.h"
class NseMqProducerCallback{
public:
    virtual void dr_msg_cb (rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque);
};

#endif //NSEMQ_CXX_PRODUCERCALLBACK_H
