ADD_EXECUTABLE(make_sample make_sample.cpp)
ADD_EXECUTABLE(make_sample16 make_sample16.cpp)
ADD_EXECUTABLE(make_sample_mtif make_sample_mtif.cpp)
ADD_EXECUTABLE(validate validate.cpp)
ADD_EXECUTABLE(validate_yzx validate_yzx.cpp)


ADD_CUSTOM_TARGET(check
                DEPENDS check8 check16 checkmtif check_custom_pitch check_verbose
        )
ADD_CUSTOM_TARGET(check_verbose
        COMMAND make_sample
        COMMAND xyz2zxy -i sample -o output_zxy -n 4 -ext ".png" --verbose
)
ADD_CUSTOM_TARGET(checkmtif
        COMMAND make_sample_mtif
        COMMAND xyz2zxy -i mtifsample.tif -o output_zxy -n 4 -ext ".tif"
        COMMAND validate output_zxy
        )
ADD_CUSTOM_TARGET(check8
        COMMAND make_sample
        COMMAND xyz2zxy -i sample -o output_zxy -n 4 -ext ".png"
        COMMAND validate output_zxy
        COMMAND xyz2yzx -i sample -o output_yzx -n 4 -ext ".png"
        COMMAND validate_yzx output_yzx
        DEPENDS make_sample xyz2zxy validate
        )
ADD_CUSTOM_TARGET(check16
        COMMAND make_sample16
        COMMAND xyz2zxy -i sample16 -o output16 -n 8 -ext ".tif"
        COMMAND validate output16
        DEPENDS xyz2zxy make_sample16 validate
        )
ADD_CUSTOM_TARGET(check_custom_pitch
        COMMAND make_sample
        COMMAND xyz2zxy -i sample -o output_cp -n 4 -p 1 3 -ext ".png"
        )

if(BUILD_TESTING)
    add_executable(buffer_regression buffer_regression.cpp)
    set(regression_dir "${CMAKE_CURRENT_BINARY_DIR}/regression")
    add_test(NAME buffers_prepare COMMAND buffer_regression prepare "${regression_dir}")
    set_tests_properties(buffers_prepare PROPERTIES FIXTURES_SETUP image_buffers)
    # Cover 8/16 bit, one/three/four channels.
    foreach(type 8UC1 8UC3 8UC4 16UC1 16UC3 16UC4)
        foreach(ext png tif)
            foreach(step 1 3 20)
                foreach(axis zxy yzx)
                    set(name "buffers_${type}_${ext}_${step}_${axis}")
                    set(output "${regression_dir}/${name}.${ext}")
                    add_test(NAME ${name} COMMAND xyz2${axis}
                        -i "${regression_dir}/${type}.${ext}" -o "${output}" -n ${step} -ext ".${ext}")
                    set_tests_properties(${name} PROPERTIES FIXTURES_REQUIRED image_buffers)
                    add_test(NAME ${name}_verify COMMAND buffer_regression verify "${output}" ${type} ${axis})
                    set_tests_properties(${name}_verify PROPERTIES DEPENDS ${name} FIXTURES_REQUIRED image_buffers)
                endforeach()
            endforeach()
        endforeach()
        foreach(axis zxy yzx)
            set(name "buffers_mtif_${type}_${axis}")
            set(output "${regression_dir}/${name}.tif")
            add_test(NAME ${name} COMMAND xyz2${axis}
                -i "${regression_dir}/${type}.tifstack/input.tif" -o "${output}" -n 3 -ext .tif)
            set_tests_properties(${name} PROPERTIES FIXTURES_REQUIRED image_buffers)
            add_test(NAME ${name}_verify COMMAND buffer_regression verify "${output}" ${type} ${axis})
            set_tests_properties(${name}_verify PROPERTIES DEPENDS ${name} FIXTURES_REQUIRED image_buffers)
        endforeach()
    endforeach()
endif()
