CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(xyz2zxy VERSION 1.0.0 LANGUAGES CXX)
#SET(CMAKE_VERBOSE_MAKEFILE 1)
#SET(CMAKE_INSTALL_PREFIX "./ex")
# BUILD mode
#SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_BUILD_TYPE Release)
#SET(CMAKE_BUILD_TYPE RelWithDebInfo)
#SET(CMAKE_BUILD_TYPE MinSizeRel)
####################
#### C++ options
####################
SET(CMAKE_CXX_STANDARD 17) #11, 17
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)
# Libraries
IF(MSVC)
    SET(OpenCV_DIR "C:/opencv/build") #directory where OpenCVConfig.cmake exists
    SET(CMAKE_PREFIX_PATH, "c:/fmt-8.1.1")
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
##
## Executables.
##
configure_file(Version.hpp.in Version.hpp)
ADD_EXECUTABLE(xyz2zxy xyz2zxy_main.cpp Xyz2ZxyProgram.hpp Xyz2ZxyProgram.cpp)
TARGET_INCLUDE_DIRECTORIES(xyz2zxy PRIVATE ${OpenCV_INCLUDE_DIRS} . ${CMAKE_BINARY_DIR})
TARGET_LINK_LIBRARIES(xyz2zxy PRIVATE ${OpenCV_LIBS} fmt::fmt-header-only Threads::Threads)

ADD_EXECUTABLE(make_sample make_sample.cpp)
TARGET_INCLUDE_DIRECTORIES(make_sample PRIVATE ${OpenCV_INCLUDE_DIRS} .)
TARGET_LINK_LIBRARIES(make_sample PRIVATE ${OpenCV_LIBS} fmt::fmt-header-only Threads::Threads)

ADD_EXECUTABLE(validate validate.cpp)
TARGET_INCLUDE_DIRECTORIES(validate PRIVATE ${OpenCV_INCLUDE_DIRS} .)
TARGET_LINK_LIBRARIES(validate PRIVATE ${OpenCV_LIBS} fmt::fmt-header-only Threads::Threads)

install(TARGETS xyz2zxy
        CONFIGURATIONS Release
        RUNTIME DESTINATION bin
        )
##
## Validation script
##
ADD_CUSTOM_TARGET(check
        COMMAND make_sample
        COMMAND xyz2zxy -i sample -o output -n 4
        COMMAND validate output
        DEPENDS make_sample xyz2zxy validate
        )
