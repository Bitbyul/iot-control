cmake_minimum_required(VERSION 3.0.0)
project(iot-client VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

add_executable(${PROJECT_NAME} 
        main.cpp
        session.cpp session.h
        controller.cpp controller.h
        devices/DeviceBuilder.h devices/DeviceBuilder.cpp devices/DeviceEntry.h devices/DeviceEntry.cpp 
        tools/logger.cpp tools/logger.h tools/datatype.cpp tools/datatype.h )

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
