CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(xyz2zxy CXX)
#set(CMAKE_VERBOSE_MAKEFILE 1)
#set(CMAKE_INSTALL_PREFIX "./ex")
# BUILD mode
# set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_BUILD_TYPE Release)
# set(CMAKE_BUILD_TYPE RelWithDebInfo)
# set(CMAKE_BUILD_TYPE MinSizeRel)
####################
#### C++ options
####################
SET(CMAKE_CXX_STANDARD 17) #11, 17
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

# Libraries
IF(MSVC)
#    set(CMAKE_PREFIX_PATH, "path-to-opencv" "path-to-fmt")
ENDIF()
find_package(OpenCV 4.0.0 REQUIRED)
find_package(fmt REQUIRED)
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
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wconversion -Wdisabled-optimization -Wnon-virtual-dtor -Wfloat-equal -Winit-self -Winline -Wmissing-include-dirs -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-promo -Wswitch-default -Wswitch-enum -Wvariadic-macros -Wwrite-strings  -Wnon-virtual-dtor -Wold-style-cast")
    set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -pg")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG ")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 -Og -pg")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG -march=native")
endif ()

ADD_EXECUTABLE(xyz2zxy xyz2zxy_main.cpp Xyz2ZxyProgram.hpp Xyz2ZxyProgram.cpp)
TARGET_INCLUDE_DIRECTORIES(xyz2zxy PRIVATE ${OpenCV_INCLUDE_DIRS} .)
TARGET_LINK_LIBRARIES(xyz2zxy PRIVATE ${OpenCV_LIBS} fmt::fmt-header-only Threads::Threads)

ADD_EXECUTABLE(make_sample make_sample.cpp)
TARGET_INCLUDE_DIRECTORIES(make_sample PRIVATE ${OpenCV_INCLUDE_DIRS} .)
TARGET_LINK_LIBRARIES(make_sample PRIVATE ${OpenCV_LIBS} fmt::fmt-header-only Threads::Threads)

ADD_EXECUTABLE(validate validate.cpp)
TARGET_INCLUDE_DIRECTORIES(validate PRIVATE ${OpenCV_INCLUDE_DIRS} .)
TARGET_LINK_LIBRARIES(validate PRIVATE ${OpenCV_LIBS} fmt::fmt-header-only Threads::Threads)

ADD_CUSTOM_TARGET(check
        COMMAND make_sample
        COMMAND xyz2zxy -i sample -o output -n 4
        COMMAND validate output
        DEPENDS make_sample xyz2zxy validate
        )
