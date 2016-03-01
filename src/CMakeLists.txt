set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS_DEBUG "-fdiagnostics-color=always -std=gnu11 -g -O0 -Wall -Wextra -Winline -fno-common -Ddebug")
set(CMAKE_C_FLAGS_RELEASE "-fdiagnostics-color=always -std=gnu11 -g -O2 -Wall -Wextra -Winline -fno-common")

add_executable(qweb signals.c main.c error.c socket.c request.c response.c rio.c ${HEADERS})
target_include_directories(qweb PRIVATE ../http-parser)
set(HEADERS socket.h error.h signals.h request.h response.h rio.h http_parser.h)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

target_link_libraries(qweb http_parser)
cmake_policy(SET CMP0015 OLD)
link_directories(../http-parser)

