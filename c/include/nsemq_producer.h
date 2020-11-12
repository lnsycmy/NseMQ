#ifndef NSEMQ_PRODUCER_H_
#define NSEMQ_PRODUCER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "nsemq_base.h"

/**
 * initialize producer
 * @param broker_addr: the broker server address , i.e. "localhost:9092".
 * @param dr_msg_cb: deliver report callback function pointer, which defined by the user and will be auto-called internally.
 */
NSEMQ_API ErrorCode nsemq_producer_init(const char *broker_addr, dr_cb_func dr_msg_cb);

/**
 * produce message with 'msg' and topic 'topic'
 * @param msg: message to be produced, which is the struct pointer type, i.e. nse_cpx *cpx.
 * @param topic: topic to be published belong above msg.
 */
NSEMQ_API ErrorCode nsemq_producer_produce(void *msg, const char *topic);

/* close producer and clear memory */
NSEMQ_API ErrorCode nsemq_producer_close();

#ifdef __cplusplus
}
#endif
#endif //NSEMQ_PRODUCER_H_
