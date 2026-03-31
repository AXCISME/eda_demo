cmake_minimum_required(VERSION 3.10)
project(eda_demo LANGUAGES C CXX)

option(EDA_ENABLE_HTTP "Enable HTTP transport" ON)          # HTTP 能力开关
option(EDA_ENABLE_MODBUS "Enable Modbus protocol" ON)       # MODBUS 能力开关
option(EDA_HTTP_BACKEND_FAKE "Compile fake HTTP backend" ON)    # HTTP 底层实现 假服务开关
option(EDA_HTTP_BACKEND_MONGOOSE "Compile mongoose HTTP backend" ON)    # HTTP 底层实现 mongoose开关
option(EDA_MODBUS_BACKEND_FAKE "Compile fake Modbus backend" ON)    # MODBUS 底层实现 假服务开关

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT EDA_ENABLE_HTTP)
    if (EDA_HTTP_BACKEND_FAKE OR EDA_HTTP_BACKEND_MONGOOSE)
        message(FATAL_ERROR
            "HTTP backends are enabled, but EDA_ENABLE_HTTP=OFF")
    endif()
endif()

if (EDA_ENABLE_HTTP)
    if (NOT EDA_HTTP_BACKEND_FAKE AND NOT EDA_HTTP_BACKEND_MONGOOSE)
        message(FATAL_ERROR
            "EDA_ENABLE_HTTP=ON requires at least one HTTP backend")
    endif()
endif()

if (NOT EDA_ENABLE_MODBUS)
    if (EDA_MODBUS_BACKEND_FAKE)
        message(FATAL_ERROR
            "Modbus backends are enabled, but EDA_ENABLE_MODBUS=OFF")
    endif()
endif()

if (EDA_ENABLE_MODBUS)
    if (NOT EDA_MODBUS_BACKEND_FAKE)
        message(FATAL_ERROR
            "EDA_ENABLE_MODBUS=ON requires at least one Modbus backend")
    endif()
endif()

configure_file(
    src/bootstrap/BuildFeatures.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/bootstrap/BuildFeatures.h
)

add_executable(eda_demo
    src/main.cpp
    src/application/host/ApplicationHost.cpp
    src/bootstrap/AppConfigValidation.cpp
    src/bootstrap/ApplicationBootstrap.cpp
    src/bootstrap/assembly/HttpAssembly.cpp
    src/bootstrap/assembly/ModbusAssembly.cpp
    src/bootstrap/factory/HttpBackendFactory.cpp
    src/bootstrap/factory/ModbusBackendFactory.cpp
    src/infrastructure/runtime/ModbusPollingRuntime.cpp
)

target_include_directories(eda_demo PRIVATE src)
target_include_directories(eda_demo PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

if (EDA_ENABLE_HTTP AND EDA_HTTP_BACKEND_MONGOOSE)
    target_sources(eda_demo PRIVATE
        src/infrastructure/transport/http/MongooseHttpServerAdapter.cpp
        third_party/mongoose/mongoose.c
    )
    target_include_directories(eda_demo PRIVATE third_party/mongoose)
endif()

find_package(Threads REQUIRED)
target_link_libraries(eda_demo Threads::Threads)
