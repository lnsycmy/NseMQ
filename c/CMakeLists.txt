cmake_minimum_required(VERSION 3.16)
project(NseMQ_C)

set(CMAKE_C_STANDARD 11)

include_directories(../include)
link_directories("lib")
link_libraries("librdkafka.dll")

add_library(NseMQ
            src/nsemq_base.h
            src/nsemq_producer.h
            src/nsemq_producer.c)

add_executable(nsemq_producer_example examples/nsemq_producer_example.c)
target_link_libraries(nsemq_producer_example NseMQ)