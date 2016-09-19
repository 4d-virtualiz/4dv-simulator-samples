cmake_minimum_required(VERSION 2.8.9 FATAL_ERROR)

# gcc -Wall -pedantic -Wextra 

# forcing build type for debug
SET(CMAKE_BUILD_TYPE "Release")

add_compile_options(-Wall -Wextra -Werror -pedantic)

SET(TCPRAW_C_SOURCES_FILES   ${CMAKE_CURRENT_SOURCE_DIR}/src/global.c
                             ${CMAKE_CURRENT_SOURCE_DIR}/src/packet.c
                             ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.c
                             ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c

)
SET(TCPRAW_C_SOURCES  "${TCPRAW_C_SOURCES_FILES}")

ADD_EXECUTABLE(tcpRawClient ${TCPRAW_C_SOURCES})
