cmake_minimum_required(VERSION 3.16)
project(NseMQ)

#set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD 98)

include_directories("include")

link_directories("lib\\vs2015\\x64\\Debug")
link_libraries("librdkafka.dll")
link_libraries("librdkafkacpp.dll")
link_libraries("avrocpp.lib")
link_libraries("boost_filesystem-vc140-mt-gd-x64-1_72.lib
                boost_iostreams-vc140-mt-gd-x64-1_72.lib
                boost_program_options-vc140-mt-gd-x64-1_72.lib
                boost_regex-vc140-mt-gd-x64-1_72.lib
                boost_system-vc140-mt-gd-x64-1_72.lib
                boost_thread-vc140-mt-gd-x64-1_72.dll")

# boost
# set(Boost_USE_STATIC_LIBS ON)
#[[find_package(Boost REQUIRED COMPONENTS system filesystem iostreams program_options regex)
if(!Boost_FOUND)
    message("Boost NOT FOUND")
endif()
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})]]


add_library(NseMQ STATIC
        src/NseMqProducer.cpp
        src/NseMqConsumer.cpp
        src/NseMqHandle.cpp)

add_executable(NseMqProducer src/NseMqProducer.cpp)
target_link_libraries(NseMqProducer librdkafkacpp)

add_executable(NseMqConsumer src/NseMqConsumer.cpp)
target_link_libraries(NseMqConsumer librdkafkacpp)

add_executable(consumer_example examples/consumer_example.cpp)
target_link_libraries(consumer_example NseMQ)

add_executable(producer_example examples/producer_example.cpp)
target_link_libraries(producer_example NseMQ)

add_executable(boostThreadTest test/boostThreadTest.cpp)
target_link_libraries(boostThreadTest boost_thread-vc140-mt-gd-x64-1_72)
