
if("dynamic" STREQUAL "static" AND NOT WIN32)
    include(CMakeFindDependencyMacro)
    find_dependency(Threads)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/libmysql-targets.cmake)
