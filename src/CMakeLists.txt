
add_library(fiberhttp 
    fiberhttp.cpp
)

IF (WIN32)
    target_link_libraries(fiberhttp PRIVATE fiberio ${Boost_LIBRARIES} unofficial::libuv::libuv ws2_32)
ELSE()
    target_link_libraries(fiberhttp PRIVATE fiberio ${Boost_LIBRARIES} unofficial::libuv::libuv)
ENDIF()

target_compile_features(fiberhttp PUBLIC cxx_std_17)