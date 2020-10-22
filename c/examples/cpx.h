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

# ifndef CPX_H_
# define CPX_H_

# include "kaa/kaa_common_schema.h"
# include "kaa/kaa_list.h"

# ifdef __cplusplus
extern "C" {
# endif



typedef struct {
    serialize_fn serialize;
    get_size_fn  get_size;
    get_type_fn  get_type;
    destroy_fn   destroy;

    kaa_string_t * name;
    int32_t age;


} nse_person_t;

nse_person_t *nse_person_create(void);
nse_person_t *nse_person_deserialize(avro_reader_t reader);



typedef struct {
    serialize_fn serialize;
    get_size_fn  get_size;
    get_type_fn  get_type;
    destroy_fn   destroy;

    double re;
    double im;
    nse_person_t * s;
    kaa_list_t * arr;


} nse_cpx_t;

nse_cpx_t *nse_cpx_create(void);
nse_cpx_t *nse_cpx_deserialize(avro_reader_t reader);

#ifdef __cplusplus
}      /* extern "C" */
#endif
#endif