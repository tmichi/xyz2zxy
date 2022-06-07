CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(xyz2zxy VERSION 1.2.0 LANGUAGES CXX)
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
ENDIF(MSVC)
find_package(OpenCV 4.5.0 REQUIRED)
find_package(Threads REQUIRED)
if (MSVC) #Visual Studio
    set(CMAKE_CXX_FLAGS "/W4 /EHsc")
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
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS} .)
LINK_LIBRARIES(${OpenCV_LIBS} Threads::Threads)

configure_file(Version.hpp.in Version.hpp)
ADD_EXECUTABLE(xyz2zxy xyz2zxy_main.cpp Xyz2ZxyProgram.hpp Xyz2ZxyProgram.cpp)
TARGET_INCLUDE_DIRECTORIES(xyz2zxy PRIVATE ${CMAKE_BINARY_DIR})

ADD_EXECUTABLE(make_sample make_sample.cpp)
ADD_EXECUTABLE(make_sample16 make_sample16.cpp)
ADD_EXECUTABLE(validate validate.cpp)
install(TARGETS xyz2zxy
        CONFIGURATIONS Release
        RUNTIME DESTINATION bin
        )
##
## Validation script
##
ADD_CUSTOM_TARGET(check
        DEPENDS check8 check16 check_custom_pitch
        )
ADD_CUSTOM_TARGET(check8
        COMMAND make_sample
        COMMAND xyz2zxy -i sample -o output -n 4 -ext ".png"
        COMMAND validate output
        DEPENDS make_sample xyz2zxy validate
        )
ADD_CUSTOM_TARGET(check16
        COMMAND make_sample16
        COMMAND xyz2zxy -i sample16 -o output16 -n 4
        COMMAND validate output16
        DEPENDS xyz2zxy make_sample16 validate
        )
ADD_CUSTOM_TARGET(check_custom_pitch
        COMMAND make_sample
        COMMAND xyz2zxy -i sample -o output_cp -n 4 -pitch 1 3 -ext ".png"
        )
