set(CMAKE_CXX_STANDARD 14)
set(BOOST_ROOT c:/Users/dkononenko/Applications/boost_1_58_0)
set(SOURCE_FILES main.cpp)

set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED ON)

find_package(Boost 1.58 COMPONENTS system date_time regex log REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(serialproxy ${SOURCE_FILES})
    target_link_libraries(serialproxy ${Boost_LIBRARIES})
endif()
