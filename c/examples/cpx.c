/*
 * Copyright 2014-2016 CyberVision, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

# include <inttypes.h>
# include <string.h>
# include "stdio.h"
# include "cpx.h"
# include "kaa/avro_src/avro/io.h"
# include "kaa/avro_src/encoding.h"
# include "kaa/kaa_mem.h"

/*
 * AUTO-GENERATED CODE
 */



static void nse_person_destroy(void *data)
{
    if (data) {
        nse_person_t *record = (nse_person_t *)data;

        kaa_string_destroy(record->name);
        kaa_data_destroy(record);
    }
}

static void nse_person_serialize(avro_writer_t writer, void *data)
{
    if (data) {
        nse_person_t *record = (nse_person_t *)data;

        kaa_string_serialize(writer, record->name);
        kaa_int_serialize(writer, &record->age);
    }
}

static size_t nse_person_get_size(void *data)
{
    if (data) {
        size_t record_size = 0;
        nse_person_t *record = (nse_person_t *)data;

        record_size += kaa_string_get_size(record->name);
        record_size += kaa_int_get_size(&record->age);

        return record_size;
    }

    return 0;
}

static char* nse_person_get_type(void *data)
{
    if (data) {
        return "nse_person";
    }
    return "";
}

nse_person_t *nse_person_create(void)
{
    nse_person_t *record = 
            (nse_person_t *)KAA_CALLOC(1, sizeof(nse_person_t));

    if (record) {
        record->serialize = nse_person_serialize;
        record->get_size = nse_person_get_size;
        record->get_type = nse_person_get_type;
        record->destroy = nse_person_destroy;
    }

    return record;
}

nse_person_t *nse_person_deserialize(avro_reader_t reader)
{
    nse_person_t *record = 
            (nse_person_t *)KAA_MALLOC(sizeof(nse_person_t));

    if (record) {
        record->serialize = nse_person_serialize;
        record->get_size = nse_person_get_size;
        record->get_type = nse_person_get_type;
        record->destroy = nse_person_destroy;

        record->name = kaa_string_deserialize(reader);
        avro_binary_encoding.read_int(reader, &record->age);
    }

    return record;
}



static void nse_cpx_destroy(void *data)
{
    if (data) {
        nse_cpx_t *record = (nse_cpx_t *)data;

        if (record->s && record->s->destroy) {
            record->s->destroy(record->s);
        }
            kaa_list_destroy(record->arr, kaa_string_destroy);
            kaa_data_destroy(record);
    }
}

static void nse_cpx_serialize(avro_writer_t writer, void *data)
{
    if (data) {
        nse_cpx_t *record = (nse_cpx_t *)data;

        kaa_double_serialize(writer, &record->re);
        kaa_double_serialize(writer, &record->im);
        record->s->serialize(writer, record->s);
            kaa_array_serialize(writer, record->arr, kaa_string_serialize);
        }
}

static size_t nse_cpx_get_size(void *data)
{
    if (data) {
        size_t record_size = 0;
        nse_cpx_t *record = (nse_cpx_t *)data;

        record_size += AVRO_DOUBLE_SIZE;
        record_size += AVRO_DOUBLE_SIZE;
        record_size += record->s->get_size(record->s);
            record_size += kaa_array_get_size(record->arr, kaa_string_get_size);
    
        return record_size;
    }

    return 0;
}

static char* nse_cpx_get_type(void *data)
{
    if (data) {
        return "nse_cpx";
    }
    return "";
}

nse_cpx_t *nse_cpx_create(void)
{
    nse_cpx_t *record = 
            (nse_cpx_t *)KAA_CALLOC(1, sizeof(nse_cpx_t));

    if (record) {
        record->serialize = nse_cpx_serialize;
        record->get_size = nse_cpx_get_size;
        record->get_type = nse_cpx_get_type;
        record->destroy = nse_cpx_destroy;
    }

    return record;
}

nse_cpx_t *nse_cpx_deserialize(avro_reader_t reader)
{
    nse_cpx_t *record = 
            (nse_cpx_t *)KAA_MALLOC(sizeof(nse_cpx_t));

    if (record) {
        record->serialize = nse_cpx_serialize;
        record->get_size = nse_cpx_get_size;
        record->get_type = nse_cpx_get_type;
        record->destroy = nse_cpx_destroy;

        avro_binary_encoding.read_double(reader, &record->re);
        avro_binary_encoding.read_double(reader, &record->im);
        record->s = nse_person_deserialize(reader);
            record->arr = kaa_array_deserialize_wo_ctx(reader, (deserialize_wo_ctx_fn)kaa_string_deserialize);
        }

    return record;
}

