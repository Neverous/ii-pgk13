PROJECT(viewer)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(WIN32)
    INCLUDE(../win32/win32.cmake)

ELSE(WIN32)
    IF(USE_STATIC)
        INCLUDE(../static/static.cmake)

    ELSE(USE_STATIC)
        FIND_PACKAGE(PkgConfig  REQUIRED)
        FIND_PACKAGE(OpenGL     REQUIRED)
        FIND_PACKAGE(GLM        REQUIRED)
        FIND_PACKAGE(GLEW       REQUIRED)
        PKG_SEARCH_MODULE(GLFW      REQUIRED glfw3)
        PKG_SEARCH_MODULE(ASSIMP    REQUIRED assimp)

    ENDIF(USE_STATIC)

    # BASE C COMPILER FLAGS
    SET(CMAKE_C_FLAGS                       "-pipe -Wall -Wshadow -Woverflow -Wextra -Wstrict-overflow=5 -std=c11")
    SET(CMAKE_C_FLAGS_RELEASE               "-O3 -DNDEBUG")
    SET(CMAKE_C_FLAGS_DEBUG                 "-g -O0 -Werror")

    # BASE CXX COMPILER FLAGS
    SET(CMAKE_CXX_FLAGS                     "-pipe -Wall -Wshadow -Woverflow -Wextra -Wstrict-overflow=5 -std=c++11")
    SET(CMAKE_CXX_FLAGS_RELEASE             "-O3 -DNDEBUG")
    SET(CMAKE_CXX_FLAGS_DEBUG               "-g -O0 -Werror")

ENDIF(WIN32)

SET(CMAKE_EXPORT_COMPILE_COMMANDS   TRUE)

INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIRS} ${GLM_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ${GLFW_INCLUDE_DIRS} ${ASSIMP_INCLUDE_DIRS})

MESSAGE(STATUS "Using host compiler: ${CMAKE_CXX_COMPILER_ID} v${CMAKE_CXX_COMPILER_VERSION}")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/defines.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/defines.h)

ADD_SUBDIRECTORY(src/)
