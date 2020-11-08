cmake_minimum_required(VERSION 3.17)

project(fiberhttp VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

find_package(unofficial-libuv CONFIG REQUIRED)
find_package(Boost COMPONENTS fiber)

find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(libmysql REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

include_directories(include)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(libs/fiberio/include)
include_directories(libs/cpp-httplib)
include_directories(libs/http_parser)

add_subdirectory(libs/fiberio)
add_subdirectory(libs/cpp-httplib)
add_subdirectory(src)
add_subdirectory(examples)