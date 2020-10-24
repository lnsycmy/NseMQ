cmake_minimum_required(VERSION 3.16)
project(NseMQ_C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
link_directories("lib")
link_libraries("librdkafka.dll")
link_libraries("pthreadVC2.dll")

# add_definitions(-DNSEMQ_EXPORTS)
add_library(NseMQ SHARED
        include/kaa/avro_src/avro/io.h
        include/kaa/avro_src/avro/platform.h
        include/kaa/avro_src/avro_private.h
        include/kaa/avro_src/encoding.h
        src/encoding_binary.c
        src/io.c
        src/kaa_list.c
        include/kaa/kaa_list.h
        src/kaa_mem.c
        include/kaa/kaa_mem.h
        src/kaa_common_schema.c
        include/kaa/kaa_common_schema.h
        include/kaa/kaa_common.h
        include/kaa/kaa_error.h

        include/nsemq_base.h
        src/nsemq_base.c
        include/nsemq_producer.h
        src/nsemq_producer.c
        include/nsemq_consumer.h
        src/nsemq_consumer.c
        src/uuid4.c
        src/map.c
        include/nsemq.h)

add_executable(nsemq_producer_example examples/nsemq_producer_example.c examples/cpx.c)
target_link_libraries(nsemq_producer_example NseMQ)

add_executable(nsemq_consumer_example examples/nsemq_consumer_example.c examples/cpx.c)
target_link_libraries(nsemq_consumer_example NseMQ)

add_executable(nsemq_avrogen examples/nsemq_avrogen.c)
target_link_libraries(nsemq_avrogen NseMQ)

add_executable(kaa_avro_test examples/kaa_avro_test.c)
target_link_libraries(kaa_avro_test NseMQ)
