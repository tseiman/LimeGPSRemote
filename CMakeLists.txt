cmake_minimum_required(VERSION 3.10)

project(LimeGPSServer VERSION 1.0 LANGUAGES C)

add_executable(LimeGPSServer src/LimeGPSServer.c src/limegps.c src/gpssim.c src/GPSRunner.c src/ControlHttpServer.c)

target_compile_options(LimeGPSServer PUBLIC -O2)


target_link_libraries(LimeGPSServer m pthread LimeSuite microhttpd)



