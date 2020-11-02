#include <stdio.h>
#include <string.h>
#include "nsemq.h"
#include "cpx.h"

void msg_callback(void *msg_data, char *msg_topic, char *msg_type){
    printf("received a message!\n");
    if(strcmp(msg_type, "nse_cpx") == 0){
        nse_cpx_t *cpx = (nse_cpx_t *)msg_data;
        printf("cpx: im: %lf, re: %lf, name: %s, age: %d\n",
               cpx->im, cpx->re, cpx->s->name->data, cpx->s->age);
        printf("cpx msg_topic:%s\n",msg_topic);
    }else if(strcmp(msg_type, "nse_person") == 0){
        nse_person_t *person = (nse_person_t *)msg_data;
        printf("person: name: %s, age: %d\n",
               person->name->data, person->age);
    }
}

int main(){
    if(nsemq_consumer_init("localhost:9092") != ERR_NO_ERROR) {
        return -1;
    }

    nsemq_consumer_subscribe("test", nse_cpx, msg_callback);
    nsemq_consumer_subscribe("test1", nse_cpx, msg_callback);

    // get subscribed topic.
    list_t *topic_list = list_new();
    nsemq_consumer_subscriptions(topic_list);
    list_node_t *node;
    list_iterator_t *it = list_iterator_new(topic_list, LIST_HEAD);
    while ((node = list_iterator_next(it))) {
        printf("topic:%s\n", node->val);
    }
    list_destroy(topic_list);

    nsemq_consumer_close();
}
