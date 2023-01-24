CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(xyz2zxy VERSION 2.0.0 LANGUAGES CXX)
# BUILD mode
SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_CXX_STANDARD 20) #11, 17
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)
# Libraries
IF(MSVC)
    SET(OpenCV_DIR "C:/opencv/build") #directory where OpenCVConfig.cmake exists
ENDIF(MSVC)
FIND_PACKAGE(fmt 7.0.0 REQUIRED)
find_package(OpenCV 4.5.0 REQUIRED)
find_package(Threads REQUIRED)

if (APPLE)
    if (CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64")
    elseif (CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch arm64")
    endif()
elseif(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
elseif(WIN32)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
    endif()
endif()

# Add compiler flags for different build types
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /O2 /Zi /DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /O2 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -g -DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Os -DNDEBUG")
endif()
##
## Executables.
##


INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS} . ${CMAKE_BINARY_DIR})
LINK_LIBRARIES(${OpenCV_LIBS} fmt::fmt-header-only Threads::Threads)
CONFIGURE_FILE(xyz2zxy_version.hpp.in xyz2zxy_version.hpp)
CONFIGURE_FILE(README.txt.in README.txt)
ADD_EXECUTABLE(xyz2zxy xyz2zxy_main.cpp)
ADD_EXECUTABLE(xyz2yzx xyz2yzx_main.cpp xyz2zxy.hpp)
ADD_SUBDIRECTORY(tests)

ADD_CUSTOM_TARGET(check
        DEPENDS check8 check16 check_custom_pitch
        )
ADD_CUSTOM_TARGET(check8
        COMMAND make_sample
        COMMAND xyz2zxy -i sample -o output_zxy -n 4 -ext ".png"
        COMMAND validate output_zxy
        COMMAND xyz2yzx -i sample -o output_yzx -n 8 -p 10 20 -ext ".tif"
        COMMAND validate_yzx output_yzx
        DEPENDS make_sample xyz2zxy validate
        )
ADD_CUSTOM_TARGET(check16
        COMMAND make_sample16
        COMMAND xyz2zxy -i sample16 -o output16 -n 8 -ext ".png"
        COMMAND validate output16
        DEPENDS xyz2zxy make_sample16 validate
        )
ADD_CUSTOM_TARGET(check_custom_pitch
        COMMAND make_sample
        COMMAND xyz2zxy -i sample -o output_cp -n 4 -pitch 1 3 -ext ".tif"
        )
#
# Archiving by CPack
#
INSTALL(TARGETS xyz2zxy xyz2yzx RUNTIME DESTINATION bin) #プログラム
INSTALL(FILES ${CMAKE_BINARY_DIR}/README.txt DESTINATION .)
SET(CPACK_SOURCE_IGNORE_FILES cmake-*;build;.git*;.DS_Store;.idea)
set(CPACK_GENERATOR "ZIP")
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION})
include(CPack)