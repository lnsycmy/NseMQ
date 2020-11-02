cmake_minimum_required(VERSION 3.16)
project(NseMQ_C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
link_directories("lib32")
link_libraries("librdkafka.dll")
link_libraries("pthreadVC2.dll")

# add_definitions(-DNSEMQ_EXPORTS)
add_library(nsemq SHARED
        src/kaa/encoding_binary.c
        src/kaa/io.c
        src/kaa/kaa_list.c
        src/kaa/kaa_mem.c
        src/kaa/kaa_common_schema.c
        src/utils/uuid4.c
        src/utils/map.c
        src/utils/list.c
        src/nsemq_base.c
        src/nsemq_producer.c
        src/nsemq_consumer.c)

add_executable(nsemq_producer_example examples/nsemq_producer_example.c examples/cpx.c)
target_link_libraries(nsemq_producer_example nsemq)

add_executable(nsemq_consumer_example examples/nsemq_consumer_example.c examples/cpx.c)
target_link_libraries(nsemq_consumer_example nsemq)

add_executable(nsemq_consumer_subscription examples/nsemq_consumer_subscription.c examples/cpx.c)
target_link_libraries(nsemq_consumer_subscription nsemq)
