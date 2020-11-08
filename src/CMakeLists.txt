
add_library(fiberhttp 
    fiberhttp.cpp
    database.cpp
    dbresult.cpp
    ../libs/http_parser/http_parser.c
    router.cpp
    parser.cpp
    reply.cpp
    request.cpp
    driver/sqlite.cpp
    driver/mysql_driver.cpp
)

IF (WIN32)
    target_link_libraries(fiberhttp PRIVATE 
    fiberio 
    ${Boost_LIBRARIES} 
    unofficial::libuv::libuv
    unofficial::sqlite3::sqlite3
    libmysql
    fmt::fmt
    nlohmann_json
    nlohmann_json::nlohmann_json
    ws2_32
)
ELSE()
    target_link_libraries(fiberhttp PRIVATE 
    fiberio 
    ${Boost_LIBRARIES} 
    unofficial::libuv::libuv 
    unofficial::sqlite3::sqlite3 
    /media/ssd/Project/cpp/vcpkg_lin/installed/x64-linux/lib/libmysqlclient.a
    OpenSSL::SSL 
    OpenSSL::Crypto
    ZLIB::ZLIB
    #libmysql
    fmt::fmt
    nlohmann_json 
    nlohmann_json::nlohmann_json
)
ENDIF()

target_compile_features(fiberhttp PUBLIC cxx_std_17)