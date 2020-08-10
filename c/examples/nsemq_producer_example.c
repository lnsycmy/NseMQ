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
    nsemq_producer_init(broker_addr, dr_msg_cb);
    const char * topic_name = "test";
    char * data = "this is first message.";
    nsemq_producer_produce(data, topic_name);
    char * data2 = "this is second message.";
    nsemq_producer_produce(data2, topic_name);
    nsemq_producer_close();

}
