cmake_minimum_required(VERSION 3.10)

project(LimeGPSServer VERSION 1.0 LANGUAGES C)

set(U_CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
list(APPEND CMAKE_MODULE_PATH "${U_CMAKE_MODULE_PATH}")



add_executable(LimeGPSServer src/LimeGPSServer.c src/limegps.c src/gpssim.c src/GPSRunner.c src/ControlHttpServer.c)

target_compile_options(LimeGPSServer PUBLIC -O2)


target_link_libraries(LimeGPSServer m pthread LimeSuite microhttpd)


# set_property(SOURCE src/ControlHttpServer.c PROPERTY COMPILE_DEFINITIONS OLD_MHD_HANDLER="NO")
# target_compile_definitions(LimeGPSServer PUBLIC OLD_MHD_HANDLER="NO")
include(FindMHD)
find_package(MHD ${MHD_MIN_VERSION} REQUIRED)
if (MHD_FOUND)
    set(LIBS ${LIBS} ${MHD_LIBRARIES})
    include_directories(${MHD_INCLUDE_DIRS})
    if (MHD_VERSION_STRING VERSION_LESS "1.9.71")
#	target_compile_definitions(LimeGPSServer PUBLIC OLD_MHD_HANDLER="YES")
set_property(SOURCE src/ControlHttpServer.c PROPERTY COMPILE_DEFINITIONS OLD_MHD_HANDLER="YES")
    endif ()
endif ()

