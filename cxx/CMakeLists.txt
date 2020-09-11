cmake_minimum_required(VERSION 3.16)
project(NseMQ_CXX)

set(CMAKE_CXX_STANDARD 11)


include_directories("../include")

link_directories("lib")
link_libraries("librdkafka.dll")
link_libraries("avrocpp.dll")
# boost
set(Boost_ROOT "lib")
find_package(Boost 1.72.0 COMPONENTS system filesystem iostreams program_options regex thread)
if(!Boost_FOUND)
    message("Boost NOT FOUND")
endif()
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_library(NseMQ++ SHARED
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

add_executable(test_C++ examples/test.cpp)

