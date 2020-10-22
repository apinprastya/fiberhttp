
add_library(fiberhttp 
    fiberhttp.cpp
    database.cpp
    dbresult.cpp
    driver/sqlite.cpp
)

IF (WIN32)
    target_link_libraries(fiberhttp PRIVATE fiberio ${Boost_LIBRARIES} unofficial::libuv::libuv unofficial::sqlite3::sqlite3 ws2_32)
ELSE()
    target_link_libraries(fiberhttp PRIVATE fiberio ${Boost_LIBRARIES} unofficial::libuv::libuv unofficial::sqlite3::sqlite3)
ENDIF()

target_compile_features(fiberhttp PUBLIC cxx_std_17)