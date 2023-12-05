cmake_minimum_required(VERSION 3.4.1)

project(nacos-sdk-cpp)

set(CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=strict-aliasing")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

#add_definitions(-DNACOS_AUTH)
#add_definitions(-DNACOS_SPAS)

aux_source_directory( src SRCS_DIRS)
aux_source_directory( src/config SRCS_DIRS)
aux_source_directory( src/constant SRCS_DIRS)
aux_source_directory( src/crypto SRCS_DIRS)
aux_source_directory( src/crypto/base64 SRCS_DIRS)
aux_source_directory( src/crypto/hmac_sha1 SRCS_DIRS)
aux_source_directory( src/crypto/hmac_sha1/hmac SRCS_DIRS)
aux_source_directory( src/crypto/hmac_sha1/sha SRCS_DIRS)
aux_source_directory( src/crypto/md5 SRCS_DIRS)
aux_source_directory( src/factory SRCS_DIRS)
aux_source_directory( src/http SRCS_DIRS)
aux_source_directory( src/http/delegate SRCS_DIRS)
aux_source_directory( src/init SRCS_DIRS)
aux_source_directory( src/json SRCS_DIRS)
aux_source_directory( src/json/rapidjson SRCS_DIRS)
aux_source_directory( src/json/rapidjson/error SRCS_DIRS)
aux_source_directory( src/json/rapidjson/internal SRCS_DIRS)
aux_source_directory( src/json/rapidjson/msinttypes SRCS_DIRS)
aux_source_directory( src/listen SRCS_DIRS)
aux_source_directory( src/log SRCS_DIRS)
aux_source_directory( src/naming SRCS_DIRS)
aux_source_directory( src/naming/beat SRCS_DIRS)
aux_source_directory( src/naming/cache SRCS_DIRS)
aux_source_directory( src/naming/selectors SRCS_DIRS)
aux_source_directory( src/naming/subscribe SRCS_DIRS)
aux_source_directory( src/security SRCS_DIRS)
aux_source_directory( src/server SRCS_DIRS)
aux_source_directory( src/thread SRCS_DIRS)
aux_source_directory( src/utils SRCS_DIRS)

#add_library(nacos-cli SHARED ${SRCS_DIRS})
add_library(nacos-cli-static STATIC ${SRCS_DIRS})

#target_include_directories(nacos-cli
	#PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	#PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	#PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#)

target_include_directories(nacos-cli-static
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
#target_link_libraries(nacos-cli PRIVATE Threads::Threads curl z)
target_link_libraries(nacos-cli-static PRIVATE Threads::Threads curl z)
