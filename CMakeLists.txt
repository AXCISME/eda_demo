cmake_minimum_required(VERSION 3.10)
project(eda_demo LANGUAGES C CXX)

option(EDA_ENABLE_HTTP "Enable HTTP transport" ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(eda_demo
    src/main.cpp
    src/application/host/ApplicationHost.cpp
    src/bootstrap/ApplicationBootstrap.cpp
    src/infrastructure/runtime/ModbusPollingRuntime.cpp
)

target_include_directories(eda_demo PRIVATE src)

if (EDA_ENABLE_HTTP)
    target_sources(eda_demo PRIVATE
        src/infrastructure/transport/http/MongooseHttpServerAdapter.cpp
        third_party/mongoose/mongoose.c
    )
    target_include_directories(eda_demo PRIVATE third_party/mongoose)
    target_compile_definitions(eda_demo PRIVATE EDA_ENABLE_HTTP=1)
else()
    target_compile_definitions(eda_demo PRIVATE EDA_ENABLE_HTTP=0)
endif()

find_package(Threads REQUIRED)
target_link_libraries(eda_demo Threads::Threads)
