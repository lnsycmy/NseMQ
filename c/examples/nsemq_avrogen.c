
#include <stdio.h>

#include "kaa_common_schema.h"
#include "cpx.h"

char buf[1024];

int main()
{
    printf("hello world!\n");

    nse_cpx_t *cpx = nse_cpx_create();
    cpx->im = 1994;
    cpx->re = 2020;
    cpx->s = nse_person_create();
    cpx->s->name = kaa_string_copy_create("jiazequn");
    cpx->s->age = 26;
    avro_writer_t writer = avro_writer_memory(buf, 1024);
    cpx->serialize(writer, cpx);

    avro_reader_t reader = avro_reader_memory(buf, 1024);
    nse_cpx_t *cpx2 = nse_cpx_deserialize(reader);
    printf("cpx2: im: %lf, re: %lf, name: %s, age: %d\n", cpx2->im, cpx2->re, cpx2->s->name->data, cpx2->s->age);

    return 0;
}