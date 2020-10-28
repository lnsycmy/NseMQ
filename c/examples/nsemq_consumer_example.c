#include <stdio.h>
#include <string.h>
#include "nsemq.h"
#include "cpx.h"

void msg_callback1(void *msg_data, char *msg_topic, char *msg_type){
    printf("this is msg_callback1!\n");
    printf("*** msg_type:%s, msg_topic:%s\n", msg_type, msg_topic);
    printf("*** msg_data_size:%d", sizeof(msg_data));
    if(strcmp(msg_type, "nse_cpx") == 0){
        nse_cpx_t *cpx2 = (nse_cpx_t *)msg_data;
        printf("cpx2: im: %lf, re: %lf, name: %s, age: %d\n",
                cpx2->im, cpx2->re, cpx2->s->name->data, cpx2->s->age);
        printf("cpx2 msg_topic:%s\n",msg_topic);
    }else if(strcmp(msg_type, "nse_person") == 0){
        nse_person_t *person = (nse_person_t *)msg_data;
        printf("nse_person: name: %s, age: %d\n",
               person->name->data, person->age);
    }
}

void msg_callback2(void *msg_data, char *msg_topic, char *msg_type){
    printf("this is msg_callback2!\n");
}

int main(){
    int time_count = 0;

	if(nsemq_consumer_init("localhost:9092") != ERR_NO_ERROR) {
        return -1;
    }

    nsemq_consumer_subscribe("test", nse_cpx, msg_callback1);

    // get subscription
    /*int *count = 0;
    char ** arr = (char **)malloc(255 * sizeof(char *));
    nsemq_consumer_subscription(arr, &count);
    printf("count:%d\n", count);
    for(int i = 0; i < count; i++){
        printf("topic[%d]:%s\n", i, arr[i]);
    }*/
    nsemq_consumer_start();
    printf("reback main();\n");
    while(time_count < 100){
        Sleep(1000);
        time_count++;
    }
    nsemq_consumer_close();
    printf("main() end!\n");
}
