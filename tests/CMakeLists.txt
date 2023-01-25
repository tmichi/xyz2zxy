ADD_EXECUTABLE(make_sample make_sample.cpp)
ADD_EXECUTABLE(make_sample16 make_sample16.cpp)
ADD_EXECUTABLE(make_sample_mtif make_sample_mtif.cpp)
ADD_EXECUTABLE(validate validate.cpp)
ADD_EXECUTABLE(validate_yzx validate_yzx.cpp)


ADD_CUSTOM_TARGET(check
        DEPENDS check8 check16 checkmtif check_custom_pitch
        )
ADD_CUSTOM_TARGET(checkmtif
        COMMAND make_sample_mtif
        COMMAND xyz2zxy -i mtifsample.tif -o output_zxy -n 4 -ext ".png"
        COMMAND validate output_zxy
        )
ADD_CUSTOM_TARGET(check8
        COMMAND make_sample
        COMMAND xyz2zxy -i sample -o output_zxy -n 4 -ext ".png"
        COMMAND validate output_zxy
        COMMAND xyz2yzx -i sample -o output_yzx -n 8 -ext ".tif"
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
        COMMAND xyz2zxy -i sample -o output_cp -n 4 -p 1 3 -ext ".tif"
        )