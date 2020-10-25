#ifndef NSEMQ_NSEMQ_PRODUCER_H
#define NSEMQ_NSEMQ_PRODUCER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "nsemq_base.h"

NSEMQ_API ErrorCode nsemq_producer_init(const char *broker_addr, void *dr_msg_cb);  // initialize producer
NSEMQ_API ErrorCode nsemq_producer_produce(void *msg, const char *topic_name);      // produce message with 'msg' and topic 'topic_name'
NSEMQ_API ErrorCode nsemq_producer_close();                                         // close producer and clear memory

#ifdef __cplusplus
}
#endif
#endif //NSEMQ_NSEMQ_PRODUCER_H
