#ifndef NSEMQ_NSEMQ_PRODUCER_H
#define NSEMQ_NSEMQ_PRODUCER_H

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <librdkafka/rdkafka.h>
#include "nsemq_base.h"

ErrorCode nsemq_init(const char * broker_addr, void (*dr_msg_cb) (rd_kafka_t *rk,
                                                       const rd_kafka_message_t * rkmessage,
                                                       void *opaque));
ErrorCode nsemq_produce(const char * data, const char * topic_name);
ErrorCode nsemq_close();

#endif //NSEMQ_NSEMQ_PRODUCER_H
