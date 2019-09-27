cmake_minimum_required(VERSION 3.4.1)

add_subdirectory(src/main/cpp/librtmp)

add_library( native-lib
             SHARED
             src/main/cpp/native-lib.cpp
             src/main/cpp/VideoChannel.cpp)

include_directories(src/main/cpp/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/src/main/cpp/${ANDROID_ABI}")

find_library( log-lib
              log )

target_link_libraries( native-lib
                       ${log-lib}
                       rtmp
                       x264
                       log)