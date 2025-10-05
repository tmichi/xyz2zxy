ADD_EXECUTABLE(libtiff0 libtiff0.cpp)
target_include_directories(libtiff0 PRIVATE ${TIFF_INCLUDE_DIR})
target_link_libraries(libtiff0 PRIVATE ${TIFF_LIBRARIES})