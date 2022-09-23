CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(xyz2zxy VERSION 1.2.0 LANGUAGES CXX)
#SET(CMAKE_VERBOSE_MAKEFILE 1)
#SET(CMAKE_INSTALL_PREFIX "./ex")
# BUILD mode
SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_CXX_STANDARD 17) #11, 17
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)
# Libraries
IF(MSVC)
    SET(OpenCV_DIR "C:/opencv/build") #directory where OpenCVConfig.cmake exists
    SET(CMAKE_PREFIX_PATH "C:/fmt-8.1.1")
ENDIF(MSVC)

find_package(OpenCV 4.5.0 REQUIRED)
find_package(fmt 8.1.1 REQUIRED)
find_package(Threads REQUIRED)

if (MSVC) #Visual Studio
    set(CMAKE_CXX_FLAGS "/W4")
elseif (CMAKE_COMPILER_IS_GNUCXX) #UNIX
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -pg")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 -Og -pg")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG -march=native")
ELSEIF (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -pg")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG ")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 -Og -pg")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")
endif ()

INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS} . ${CMAKE_BINARY_DIR})
LINK_LIBRARIES(${OpenCV_LIBS} fmt::fmt-header-only Threads::Threads)
configure_file(xyz2zxy_version.hpp.in xyz2zxy_version.hpp)
configure_file(README.txt.in README.txt)

ADD_EXECUTABLE(xyz2zxy xyz2zxy_main.cpp)
ADD_EXECUTABLE(xyz2yzx xyz2yzx_main.cpp)
ADD_SUBDIRECTORY(tests)
ADD_CUSTOM_TARGET(check
        COMMAND make_sample
        COMMAND xyz2zxy -i sample -o output -n 8
        COMMAND validate output
        COMMAND xyz2yzx -i sample -o output_yzx -n 8
        COMMAND validate_yzx output_yzx
        COMMAND make_sample16
        COMMAND xyz2zxy -i sample16 -o output16 -n 8
        COMMAND validate output16
        DEPENDS make_sample xyz2zxy validate make_sample16 validate16
        )
#
# Archiving by CPack
#
INSTALL(TARGETS xyz2zxy xyz2yzx RUNTIME DESTINATION bin)
INSTALL(FILES ${CMAKE_BINARY_DIR}/README.txt DESTINATION .)
set(CPACK_GENERATOR "ZIP")
include(CPack)