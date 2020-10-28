cmake_minimum_required(VERSION 3.16)
project(NseMQ_C++)

set(CMAKE_CXX_STANDARD 11)

# add_definitions(-DNseMQ_EXPORTS)

include_directories("../include")

link_directories("lib\\vs2015\\x64\\Debug")
link_libraries("librdkafkacpp.dll")
link_libraries("avrocpp.dll")
# boost
set(Boost_ROOT "lib\\vs2015\\x64\\Debug")
find_package(Boost 1.72.0 COMPONENTS system filesystem iostreams program_options regex thread)
if(!Boost_FOUND)
    message("Boost NOT FOUND")
endif()
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_library(NseMQ++
        src/NseMqProducer.cpp
        src/NseMqProducer.h
        src/NseMqConsumer.cpp
        src/NseMqConsumer.h
        src/NseMqBase.cpp
        src/NseMqBase.h
        src/NseMqConsumerCallback.h)

add_executable(consumer_example examples/consumer_example.cpp)
target_link_libraries(consumer_example NseMQ++)

add_executable(producer_example examples/producer_example.cpp)
target_link_libraries(producer_example NseMQ++)
