#include <stdio.h>
#include "nsemq.h"
#include "cpx.h"

void produce_callback(const char *msg_topic, void *msg_data, int msg_size){
    printf("this is produce_callback!\n");
    fprintf(stderr,"%% Message delivered (%d bytes, topic: %s)\n",
    msg_size, msg_topic);
}

int main(){
    const char * broker_address;
	nse_person_t *person;
	nse_cpx_t *cpx;
	const char * topic_name;
	const char * topic_name2;
    kaa_list_t *array_list;
    kaa_list_node_t *iterator;

	printf("*** main() start.\n");
	broker_address = "localhost:9092";
	topic_name = "test";

    if(nsemq_producer_init(broker_address, produce_callback) != ERR_NO_ERROR){
        printf("initialize failed!\n");
        return -1;
    }
    
    cpx = nse_cpx_create();
    cpx->im = 1996;
    cpx->re = 2020;
    cpx->s = nse_person_create();
    cpx->s->name = kaa_string_copy_create("cmy");
    cpx->s->age = 24;


    array_list = kaa_list_create();
    kaa_list_push_back(array_list, kaa_string_copy_create("cmy"));
    kaa_list_push_back(array_list, kaa_string_copy_create("456"));
    kaa_list_push_back(array_list, kaa_string_copy_create("789"));

    cpx->arr = array_list;

    nsemq_producer_produce(cpx, topic_name);

    cpx->destroy(cpx);
    nsemq_producer_close();
	system("pause");
}