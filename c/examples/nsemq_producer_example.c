#include <stdio.h>
#include "nsemq.h"
#include "cpx.h"

static void dr_msg_cb (rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque) {
    if (rkmessage->err){
        fprintf(stderr, "%% Message delivery failed: %s\n",
                rd_kafka_err2str(rkmessage->err));
    }else{
        fprintf(stderr,"%% Message delivered (%d bytes, partition %"PRId32")\n",
                rkmessage->len, rkmessage->partition);
    }
}

int main(){

    printf("*** main() start.\n");

    const char * topic_name = "test1";
    const char * broker_address = "localhost:9092";

    if(nsemq_producer_init(broker_address, dr_msg_cb) != ERR_NO_ERROR){
        printf("initialize failed!\n");
        return -1;
    }

    nse_person_t *person = nse_person_create();
    person->name = kaa_string_copy_create("cmy");
    person->age = 24;

    nsemq_producer_produce(person, topic_name);
    
    nse_cpx_t *cpx = nse_cpx_create();
    cpx->im = 1996;
    cpx->re = 2020;
    cpx->s = nse_person_create();
    cpx->s->name = kaa_string_copy_create("cmy");
    cpx->s->age = 24;

    const char * topic_name2 = "test";
    nsemq_producer_produce(cpx, topic_name2);
    nsemq_producer_close();
}