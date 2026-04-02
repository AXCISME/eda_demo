cmake_minimum_required(VERSION 3.10)
project(eda_demo LANGUAGES C CXX)

option(EDA_ENABLE_HTTP "Enable HTTP transport" ON)          # HTTP 能力开关
option(EDA_ENABLE_HTTP_CLIENT "Enable HTTP client transport" ON)          # HTTP client 能力开关
option(EDA_ENABLE_MODBUS_MASTER "Enable Modbus Master protocol" ON)       # MODBUS_MASTER 能力开关
option(EDA_HTTP_BACKEND_FAKE "Compile fake HTTP backend" ON)    # HTTP 底层实现 假服务装配开关
option(EDA_HTTP_BACKEND_MONGOOSE "Compile mongoose HTTP backend" ON)    # HTTP 底层实现 mongoose装配开关
option(EDA_HTTP_CLIENT_BACKEND_FAKE "Compile fake HTTP client backend" ON)    # HTTP client 底层实现 假服务装配开关
option(EDA_MODBUS_MASTER_BACKEND_FAKE "Compile fake Modbus Master backend" ON)    # MODBUS_MASTER 底层实现 假服务装配开关
option(EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_TCP "Compile libmodbus Modbus Master TCP backend" ON)  # MODBUS_MASTER_TCP 底层实现 libmodbus装配开关
option(EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_RTU "Compile libmodbus Modbus Master RTU backend" ON)  # MODBUS_MASTER_RTU 底层实现 libmodbus装配开关

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

if (NOT EDA_ENABLE_HTTP_CLIENT)
    if (EDA_HTTP_CLIENT_BACKEND_FAKE)
        message(FATAL_ERROR
            "HTTP client backends are enabled, but EDA_ENABLE_HTTP_CLIENT=OFF")
    endif()
endif()

if (EDA_ENABLE_HTTP_CLIENT)
    if (NOT EDA_HTTP_CLIENT_BACKEND_FAKE)
        message(FATAL_ERROR
            "EDA_ENABLE_HTTP_CLIENT=ON requires at least one HTTP client backend")
    endif()
endif()

if (NOT EDA_ENABLE_MODBUS_MASTER)
    if (EDA_MODBUS_MASTER_BACKEND_FAKE
        OR EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_TCP
        OR EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_RTU)
        message(FATAL_ERROR
            "Modbus Master backends are enabled, but EDA_ENABLE_MODBUS_MASTER=OFF")
    endif()
endif()

if (EDA_ENABLE_MODBUS_MASTER)
    if (NOT EDA_MODBUS_MASTER_BACKEND_FAKE
        AND NOT EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_TCP
        AND NOT EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_RTU)
        message(FATAL_ERROR
            "EDA_ENABLE_MODBUS_MASTER=ON requires at least one Modbus Master backend")
    endif()
endif()

set(EDA_REQUIRE_LIBMODBUS OFF)
if (EDA_ENABLE_MODBUS_MASTER AND (EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_TCP OR EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_RTU))
    set(EDA_REQUIRE_LIBMODBUS ON)
endif()

set(EDA_VENDORED_LIBMODBUS_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libmodbus/include")
set(EDA_VENDORED_LIBMODBUS_LIBRARY
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libmodbus/lib/libmodbus.a")

if (EDA_REQUIRE_LIBMODBUS)
    if (EXISTS "${EDA_VENDORED_LIBMODBUS_INCLUDE_DIR}/modbus/modbus.h"
        AND EXISTS "${EDA_VENDORED_LIBMODBUS_LIBRARY}")
        set(EDA_LIBMODBUS_INCLUDE_DIR "${EDA_VENDORED_LIBMODBUS_INCLUDE_DIR}")
        set(EDA_LIBMODBUS_LIBRARY "${EDA_VENDORED_LIBMODBUS_LIBRARY}")
        message(STATUS "Using vendored libmodbus from third_party/libmodbus")
    else()
        find_path(EDA_LIBMODBUS_INCLUDE_DIR NAMES modbus/modbus.h modbus.h)
        find_library(EDA_LIBMODBUS_LIBRARY NAMES modbus)

        if (EDA_LIBMODBUS_INCLUDE_DIR AND EDA_LIBMODBUS_LIBRARY)
            message(STATUS "Using system libmodbus")
        endif()
    endif()

    if (NOT EDA_LIBMODBUS_INCLUDE_DIR OR NOT EDA_LIBMODBUS_LIBRARY)
        message(FATAL_ERROR
            "libmodbus backend requested, but libmodbus headers/library were not found")
    endif()
endif()

configure_file(
    core/bootstrap/BuildFeatures.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/bootstrap/BuildFeatures.h
)

add_executable(eda_demo
    main.cpp
    core/application/host/ApplicationHost.cpp
    core/bootstrap/AppConfigValidation.cpp
    core/bootstrap/ApplicationBootstrap.cpp
    core/bootstrap/assembly/HttpAssembly.cpp
    core/bootstrap/assembly/HttpClientAssembly.cpp
    core/bootstrap/assembly/ModbusMasterAssembly.cpp
    core/bootstrap/factory/HttpBackendFactory.cpp
    core/bootstrap/factory/HttpClientBackendFactory.cpp
    core/bootstrap/factory/ModbusMasterBackendFactory.cpp
    demo/timer_test/TimerDemoLauncher.cpp
    core/infrastructure/runtime/ModbusPollingRuntime.cpp
)

target_include_directories(eda_demo PRIVATE core)
target_include_directories(eda_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(eda_demo PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

if (EDA_ENABLE_HTTP AND EDA_HTTP_BACKEND_MONGOOSE)
    target_sources(eda_demo PRIVATE
        core/infrastructure/transport/http/MongooseHttpServerAdapter.cpp
        third_party/mongoose/mongoose.c
    )
    target_include_directories(eda_demo PRIVATE third_party/mongoose)
endif()

if (EDA_ENABLE_MODBUS_MASTER AND EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_TCP)
    target_sources(eda_demo PRIVATE
        core/infrastructure/transport/modbus/LibmodbusMasterTcpAdapter.cpp
    )
endif()

if (EDA_ENABLE_MODBUS_MASTER AND EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_RTU)
    target_sources(eda_demo PRIVATE
        core/infrastructure/transport/modbus/LibmodbusMasterRtuAdapter.cpp
    )
endif()

if (EDA_REQUIRE_LIBMODBUS)
    target_include_directories(eda_demo PRIVATE ${EDA_LIBMODBUS_INCLUDE_DIR})
    target_link_libraries(eda_demo PRIVATE ${EDA_LIBMODBUS_LIBRARY})
endif()

find_package(Threads REQUIRED)
target_link_libraries(eda_demo PRIVATE Threads::Threads)
