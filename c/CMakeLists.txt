cmake_minimum_required(VERSION 3.16)
project(NseMQ_C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
link_directories("lib")
link_libraries("librdkafka.dll")
link_libraries("pthreadVC2.dll")

add_definitions(-DNSEMQ_EXPORTS)
add_library(nsemq SHARED
        src/encoding_binary.c
        src/io.c
        src/kaa_list.c
        src/kaa_mem.c
        src/kaa_common_schema.c
        src/uuid4.c
        src/map.c
        src/nsemq_base.c
        src/nsemq_producer.c
        src/nsemq_consumer.c)

#[[add_executable(nsemq_producer_example examples/nsemq_producer_example.c examples/cpx.c)
target_link_libraries(nsemq_producer_example NseMQ)

add_executable(nsemq_consumer_example examples/nsemq_consumer_example.c examples/cpx.c)
target_link_libraries(nsemq_consumer_example NseMQ)

add_executable(nsemq_avrogen examples/nsemq_avrogen.c)
target_link_libraries(nsemq_avrogen NseMQ)

add_executable(kaa_avro_test examples/kaa_avro_test.c)
target_link_libraries(kaa_avro_test NseMQ)]]
