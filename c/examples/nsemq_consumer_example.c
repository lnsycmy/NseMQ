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
    int time_count = 0;

	if(nsemq_consumer_init("localhost:9092") != ERR_NO_ERROR) {
        return -1;
    }

    nsemq_consumer_subscribe("test", nse_cpx, msg_callback);
    nsemq_consumer_subscribe("test1", nse_cpx, msg_callback);

    nsemq_consumer_start(NSEMQ_ASYNC);
    printf("reback main();\n");
    while(time_count < 100){
        if(time_count == 10){
            nsemq_consumer_subscribe("test2", nse_cpx, msg_callback);
        }else if(time_count == 20){
            nsemq_consumer_unsubscribe("test");
        }
        Sleep(1000);
        time_count++;
    }
    nsemq_consumer_stop();
    printf("main() end!\n");
}
