//
// Created by cmy on 2020/9/27.
//

#include <stdio.h>
#include "cpx.h"
#include "kaa_common_schema.h"
#include "../include/nsemq_base.h"
int main() {
    printf("hello cmy\n");
    char *msg_buf;
    char *msg_type;

    nse_cpx_t *cpx = nse_cpx_create();
    cpx->im = 1994;
    cpx->re = 2020;
    cpx->s = nse_person_create();
    cpx->s->name = kaa_string_copy_create("cmy");
    cpx->s->age = 26;

    int msg_len = nsemq_encode(cpx, &msg_buf, &msg_type);

    avro_reader_t reader = avro_reader_memory(msg_buf, msg_len);
    nse_cpx_t *cpx2 = nse_cpx_deserialize(reader);
    printf("cpx2: im: %lf, re: %lf, name: %s, age: %d\n", cpx2->im, cpx2->re, cpx2->s->name->data, cpx2->s->age);
    printf("cpx2 type:%s", cpx2->get_type(cpx2));
    free(msg_buf);
    return 0;
}
