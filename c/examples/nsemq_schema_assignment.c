#include <stdio.h>
#include "cpx.h"

void test_array_list(){
    /* test array */
    kaa_list_t *array_list;
    kaa_list_node_t *iterator;
    array_list = kaa_list_create();
    kaa_list_push_back(array_list, "123");
    kaa_list_push_back(array_list, "456");
    kaa_list_push_back(array_list, "789");

    iterator = kaa_list_begin(array_list);
    while (iterator){
        printf("%s\n", kaa_list_get_data(iterator));
        iterator = kaa_list_next(iterator);
    }
    kaa_list_destroy(array_list, kaa_data_destroy);
}

int main(){
    test_array_list();
}