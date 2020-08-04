#include <stdio.h>
#include "../src/nsemq_producer.h"

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

    /* The rkmessage is destroyed automatically by librdkafka */
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
    nsemq_init(broker_addr,dr_msg_cb);
    const char * topic_name = "test";
    for(int i=0; i<100; i++){
        char * data = "this is a message";
        nsemq_produce(data, topic_name);
    }
    nsemq_close();
}
