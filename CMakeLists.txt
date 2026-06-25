cmake_minimum_required(VERSION 3.10)
project(eda_demo LANGUAGES C CXX)

option(EDA_ENABLE_HTTP "Enable HTTP transport" ON)
option(EDA_ENABLE_HTTP_CLIENT "Enable HTTP client transport" ON)
option(EDA_ENABLE_MODBUS_MASTER "Enable Modbus Master protocol" ON)
option(EDA_HTTP_BACKEND_FAKE "Compile fake HTTP backend" ON)
option(EDA_HTTP_BACKEND_MONGOOSE "Compile mongoose HTTP backend" ON)
option(EDA_HTTP_CLIENT_BACKEND_FAKE "Compile fake HTTP client backend" ON)
option(EDA_HTTP_CLIENT_BACKEND_MONGOOSE "Compile mongoose HTTP client backend" ON)
option(EDA_MODBUS_MASTER_BACKEND_FAKE "Compile fake Modbus Master backend" ON)
option(EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_TCP "Compile libmodbus Modbus Master TCP backend" ON)
option(EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_RTU "Compile libmodbus Modbus Master RTU backend" ON)
option(EDA_BUILD_LIBRARY "Build eda_core static library" ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── 配置校验 ──────────────────────────────────────────────────────────

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
    if (EDA_HTTP_CLIENT_BACKEND_FAKE OR EDA_HTTP_CLIENT_BACKEND_MONGOOSE)
        message(FATAL_ERROR
            "HTTP client backends are enabled, but EDA_ENABLE_HTTP_CLIENT=OFF")
    endif()
endif()

if (EDA_ENABLE_HTTP_CLIENT)
    if (NOT EDA_HTTP_CLIENT_BACKEND_FAKE AND NOT EDA_HTTP_CLIENT_BACKEND_MONGOOSE)
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

# ── libmodbus ──────────────────────────────────────────────────────────

set(EDA_REQUIRE_LIBMODBUS OFF)
if (EDA_ENABLE_MODBUS_MASTER AND (EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_TCP OR EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_RTU))
    set(EDA_REQUIRE_LIBMODBUS ON)
endif()

set(EDA_VENDORED_LIBMODBUS_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libmodbus/include")
set(EDA_VENDORED_LIBMODBUS_LIBRARY_ARCH
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libmodbus/lib/libmodbus_${CMAKE_SYSTEM_PROCESSOR}.a")
set(EDA_VENDORED_LIBMODBUS_LIBRARY_GENERIC
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libmodbus/lib/libmodbus.a")

if (EDA_REQUIRE_LIBMODBUS)
    if (EXISTS "${EDA_VENDORED_LIBMODBUS_INCLUDE_DIR}/modbus/modbus.h")
        if (EXISTS "${EDA_VENDORED_LIBMODBUS_LIBRARY_ARCH}")
            set(EDA_VENDORED_LIBMODBUS_LIBRARY "${EDA_VENDORED_LIBMODBUS_LIBRARY_ARCH}")
        elseif (EXISTS "${EDA_VENDORED_LIBMODBUS_LIBRARY_GENERIC}")
            set(EDA_VENDORED_LIBMODBUS_LIBRARY "${EDA_VENDORED_LIBMODBUS_LIBRARY_GENERIC}")
        else()
            unset(EDA_VENDORED_LIBMODBUS_LIBRARY)
        endif()
    else()
        unset(EDA_VENDORED_LIBMODBUS_LIBRARY)
    endif()

    if (DEFINED EDA_VENDORED_LIBMODBUS_LIBRARY)
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

# ── BuildFeatures.h ────────────────────────────────────────────────────

configure_file(
    core/bootstrap/BuildFeatures.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/bootstrap/BuildFeatures.h
)

# ── eda_core 静态库 ────────────────────────────────────────────────────

if (EDA_BUILD_LIBRARY)
    add_library(eda_core STATIC
        core/application/host/ApplicationHost.cpp
        core/bootstrap/AppConfigValidation.cpp
        core/bootstrap/ApplicationBootstrap.cpp
        core/bootstrap/assembly/HttpAssembly.cpp
        core/bootstrap/assembly/HttpClientAssembly.cpp
        core/bootstrap/assembly/ModbusMasterAssembly.cpp
        core/bootstrap/factory/HttpBackendFactory.cpp
        core/bootstrap/factory/HttpClientBackendFactory.cpp
        core/bootstrap/factory/ModbusMasterBackendFactory.cpp
        core/infrastructure/runtime/ModbusDeviceRuntime.cpp
        core/infrastructure/runtime/ModbusPollingRuntime.cpp
    )

    target_include_directories(eda_core PUBLIC
        core
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/generated
    )

    find_package(Threads REQUIRED)
    target_link_libraries(eda_core PUBLIC Threads::Threads -lpthread)

    if (EDA_ENABLE_HTTP AND EDA_HTTP_BACKEND_MONGOOSE)
        target_sources(eda_core PRIVATE
            core/infrastructure/transport/http/MongooseHttpServerAdapter.cpp
        )
    endif()

    if (EDA_ENABLE_MODBUS_MASTER AND EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_TCP)
        target_sources(eda_core PRIVATE
            core/infrastructure/transport/modbus/LibmodbusMasterTcpAdapter.cpp
        )
    endif()

    if (EDA_ENABLE_MODBUS_MASTER AND EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_RTU)
        target_sources(eda_core PRIVATE
            core/infrastructure/transport/modbus/LibmodbusMasterRtuAdapter.cpp
        )
    endif()

    if (EDA_ENABLE_HTTP_CLIENT AND EDA_HTTP_CLIENT_BACKEND_MONGOOSE)
        target_sources(eda_core PRIVATE
            core/infrastructure/transport/http/MongooseHttpClientAdapter.cpp
        )
    endif()

    if ((EDA_ENABLE_HTTP AND EDA_HTTP_BACKEND_MONGOOSE)
        OR (EDA_ENABLE_HTTP_CLIENT AND EDA_HTTP_CLIENT_BACKEND_MONGOOSE))
        target_sources(eda_core PRIVATE
            third_party/mongoose/mongoose.c
        )
        target_include_directories(eda_core PUBLIC third_party/mongoose)
    endif()

    if (EDA_REQUIRE_LIBMODBUS)
        target_include_directories(eda_core PUBLIC ${EDA_LIBMODBUS_INCLUDE_DIR})
        target_link_libraries(eda_core PUBLIC ${EDA_LIBMODBUS_LIBRARY})
    endif()
endif()

# ── eda_demo 可执行文件 ────────────────────────────────────────────────

if (EDA_BUILD_LIBRARY)
    add_executable(eda_demo
        main.cpp
        demo/timer_test/TimerDemoLauncher.cpp
    )
    target_link_libraries(eda_demo PRIVATE eda_core)
else()
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
        core/infrastructure/runtime/ModbusDeviceRuntime.cpp
    )

    target_include_directories(eda_demo PRIVATE core)
    target_include_directories(eda_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_include_directories(eda_demo PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

    if (EDA_ENABLE_HTTP AND EDA_HTTP_BACKEND_MONGOOSE)
        target_sources(eda_demo PRIVATE
            core/infrastructure/transport/http/MongooseHttpServerAdapter.cpp
        )
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

    if (EDA_ENABLE_HTTP_CLIENT AND EDA_HTTP_CLIENT_BACKEND_MONGOOSE)
        target_sources(eda_demo PRIVATE
            core/infrastructure/transport/http/MongooseHttpClientAdapter.cpp
        )
    endif()

    if ((EDA_ENABLE_HTTP AND EDA_HTTP_BACKEND_MONGOOSE)
        OR (EDA_ENABLE_HTTP_CLIENT AND EDA_HTTP_CLIENT_BACKEND_MONGOOSE))
        target_sources(eda_demo PRIVATE
            third_party/mongoose/mongoose.c
        )
        target_include_directories(eda_demo PRIVATE third_party/mongoose)
    endif()

    if (EDA_REQUIRE_LIBMODBUS)
        target_include_directories(eda_demo PRIVATE ${EDA_LIBMODBUS_INCLUDE_DIR})
        target_link_libraries(eda_demo PRIVATE ${EDA_LIBMODBUS_LIBRARY})
    endif()

    find_package(Threads REQUIRED)
    target_link_libraries(eda_demo PRIVATE Threads::Threads -lpthread)
endif()
