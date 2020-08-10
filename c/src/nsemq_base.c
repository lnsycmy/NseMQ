#include "nsemq_base.h"

BOOL nsemq_judge_connect(rd_kafka_t *handle){
    const struct rd_kafka_metadata *metadata;
    rd_kafka_resp_err_t err = rd_kafka_metadata(handle, 1, NULL, &metadata, 1000);
    return (err ==  RD_KAFKA_RESP_ERR_NO_ERROR)?TRUE:FALSE;
}

void nsemq_write_error(char *errstr_){
    fprintf(stderr,"%% [NseMQ][ERR] %s\n", errstr_);
}

void nsemq_write_info(char *infostr){
    fprintf(stderr,"%% [NseMQ][INFO] %s\n", infostr);
}
