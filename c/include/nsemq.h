#ifndef NSEMQ_C_NSEMQ_H
#define NSEMQ_C_NSEMQ_H

#ifdef __cplusplus
extern "C" {
#endif

#define NSEMQ_VERSION "1.0"
/* nsemq defined header file */
#include "nsemq_base.h"
#include "nsemq_producer.h"
#include "nsemq_consumer.h"
/* kaa  header file */
#include "kaa/kaa_common_schema.h"
#include "kaa/kaa_common.h"
#include "kaa/kaa_error.h"
/* librdkafka all header file */
#include "librdkafka/rdkafka.h"

#ifdef __cplusplus
}
#endif
#endif //NSEMQ_C_NSEMQ_H
