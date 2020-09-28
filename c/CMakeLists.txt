cmake_minimum_required(VERSION 3.16)
project(NseMQ_C)

set(CMAKE_C_STANDARD 11)

include_directories(../include)
include_directories(kaa)
link_directories("lib")
link_libraries("librdkafka.dll")
link_libraries("pthreadVC2.dll")

add_library(NseMQ
            src/nsemq_base.h
            src/nsemq_base.c
            src/nsemq_producer.h
            src/nsemq_producer.c
            src/nsemq_consumer.h
            src/nsemq_consumer.c)

add_library(avrogen
            kaa/avro_src/avro/io.h
            kaa/avro_src/avro/platform.h
            kaa/avro_src/avro_private.h
            kaa/avro_src/encoding.h
            kaa/avro_src/encoding_binary.c
            kaa/avro_src/io.c
            kaa/collections/kaa_list.c
            kaa/collections/kaa_list.h
            kaa/utilities/kaa_mem.c
            kaa/utilities/kaa_mem.h
            kaa/kaa_common_schema.c
            kaa/kaa_common_schema.h
            kaa/kaa_common.h
            kaa/kaa_error.h
)

add_executable(nsemq_producer_example examples/nsemq_producer_example.c examples/cpx.c)
target_link_libraries(nsemq_producer_example NseMQ avrogen)

add_executable(nsemq_consumer_example examples/nsemq_consumer_example.c examples/cpx.c)
target_link_libraries(nsemq_consumer_example NseMQ avrogen)

add_executable(nsemq_avrogen examples/nsemq_avrogen.c examples/cpx.c)
target_link_libraries(nsemq_avrogen avrogen)

add_executable(kaa_avro_test examples/kaa_avro_test.c examples/cpx.c)
target_link_libraries(kaa_avro_test avrogen NseMQ)
