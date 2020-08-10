#ifndef NSEMQ_NSEMQ_PRODUCER_H
#define NSEMQ_NSEMQ_PRODUCER_H

#include <stdio.h>
#include "nsemq_base.h"

ErrorCode nsemq_producer_init(const char * broker_addr, void *dr_msg_cb);
ErrorCode nsemq_producer_produce(char *msg, const char * topic_name);
ErrorCode nsemq_producer_close();

#endif //NSEMQ_NSEMQ_PRODUCER_H
