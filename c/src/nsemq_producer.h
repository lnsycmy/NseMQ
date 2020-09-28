#ifndef NSEMQ_NSEMQ_PRODUCER_H
#define NSEMQ_NSEMQ_PRODUCER_H

#include <stdio.h>
#include "nsemq_base.h"


#ifdef __cplusplus
extern "C" {
#endif

ErrorCode nsemq_producer_init(const char *broker_addr, void *dr_msg_cb);

ErrorCode nsemq_producer_produce(void *msg, const char *topic_name);

ErrorCode nsemq_producer_close();

#ifdef __cplusplus
}
#endif
#endif //NSEMQ_NSEMQ_PRODUCER_H
