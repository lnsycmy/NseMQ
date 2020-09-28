#include <stdio.h>
#include "../src/nsemq_producer.h"

#include "kaa_common.h"
#include "kaa_common_schema.h"
#include "cpx.h"

static void dr_msg_cb (rd_kafka_t *rk,
                       const rd_kafka_message_t *rkmessage, void *opaque) {
    if (rkmessage->err)
        fprintf(stderr, "%% Message delivery failed: %s\n",
                rd_kafka_err2str(rkmessage->err));
    else
        fprintf(stderr,
                "%% Message delivered (%zd bytes, "
                "partition %"PRId32")\n",
                rkmessage->len, rkmessage->partition);
}

char* join_char(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    if (result == NULL) exit (1);

    strcpy(result, s1);
    strcat(result, s2);

    return result;
}

int main(){
    const char * broker_addr = "localhost:9092";
    printf("*** main() start.\n");

    if(nsemq_producer_init(broker_addr, dr_msg_cb) != ERR_NO_ERROR){
        printf("initialize failed!\n");
        return -1;
    }

    nse_cpx_t *cpx = nse_cpx_create();
    cpx->im = 1996;
    cpx->re = 2020;
    cpx->s = nse_person_create();
    cpx->s->name = kaa_string_copy_create("cmy");
    cpx->s->age = 24;

    /*const char * topic_name = "test";
    nsemq_producer_produce(cpx, topic_name);*/


    const char * topic_name2 = "test1";
    nse_person_t *person = nse_person_create();
    person->name = kaa_string_copy_create("cmy2");
    person->age = 24;
    nsemq_producer_produce(person, topic_name2);

    nsemq_producer_close();
}