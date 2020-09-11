cmake_minimum_required(VERSION 3.16)
project(NseMQ_C)

set(CMAKE_C_STANDARD 11)

include_directories(../include)
link_directories("lib")
link_libraries("librdkafka.dll")
link_libraries("pthreadVC2.dll")
link_libraries("avro.lib")

add_library(NseMQ
            src/nsemq_base.h
            src/nsemq_base.c
            src/nsemq_producer.h
            src/nsemq_producer.c
            src/nsemq_consumer.h
            src/nsemq_consumer.c)

add_executable(nsemq_producer_example examples/nsemq_producer_example.c)
target_link_libraries(nsemq_producer_example NseMQ)

add_executable(nsemq_consumer_example examples/nsemq_consumer_example.c)
target_link_libraries(nsemq_consumer_example NseMQ)
