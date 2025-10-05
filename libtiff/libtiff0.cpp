//
// Created by Takashi Michikawa on 2025/10/05.
//
#include <iostream>
#include <fstream>
#include <mutex>
#include <filesystem>
#include <vector>
#include <tiffio.h>
#include <mi/Argument.hpp>
#include <xyz2zxy.hpp>
int main(int argc, char** argv) {
        std::mutex mtx;
        mi::Argument arg(argc, argv);
        std::filesystem::path input_dir;
        std::filesystem::path outputDir;
        int step = 100;
        std::filesystem::path extension = ".tif";
        std::vector<int> params;
        xyz2zxy::init_arguments("xyz2yzx_tif", arg, input_dir, outputDir, step, extension, params);

        if (extension != ".tif") {
                std::cerr << "Input file is not a tiff file" << std::endl;
                return EXIT_FAILURE;
        }

        std::filesystem::path tmpDir = outputDir.string() + "_temp";
        xyz2zxy::create_directory(tmpDir);

        std::vector<std::filesystem::path> image_paths = xyz2zxy::list_files(input_dir, tmpDir, extension);

        xyz2zxy::create_directory(outputDir);
        auto get_tmp_filename = [&tmpDir, &extension](const uint32_t y, const uint32_t z) {
                return std::format("{}/{}/image-{:05d}{}", tmpDir.string(), z, y, extension.string());
        };
        step = 1;

        // get volume size
        auto [sx, sy, sz] = xyz2zxy::get_volume_size(image_paths);

        for (uint32_t z = 0; z < sz; z += 1) {
                void* image;
                TIFF* tiff = TIFFOpen(image_paths[z].string().c_str(), "r");
                if (tif) {
                        uint32_t width, height;
                        uint16_t samplesPerPixel, bitsPerSample;
                        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
                        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
                        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
                        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
                        std::cerr<<"width "<<width<<" height "<<height<<std::endl;
                        std::cerr<<"samplesPerPixel "<<samplesPerPixel<<std::endl;
                        std::cerr<<"bitsPerSample "<<bitsPerSample<<std::endl;

                        size_t npixels = width * height;
                        void* raster = nullptr;

                const uint32_t bytePerSample = bitsPerSample / 8;
                if ( 0 < bytePerSample && bytePerSample < 8 && bitsPerSample % 8 == 0 ) {
                        raster = _TIFFmalloc(npixels * samplesPerPixel * bytePerSample);
                }
                if (!raster) {
                        fprintf(stderr, "Memory allocation failed\n");
                        TIFFClose(tif);
                        return 1;
                }

                // スキャンラインごとに読み込み
                for (uint32_t row = 0; row < height; row++) {
                        void* buf = (uint8_t*)raster + row * width * samplesPerPixel * bytePerSample;
                        if (TIFFReadScanline(tif, buf, row, 0) < 0) {
                                std::cerr<<"Failed to read scanline"<<std::endl;
                                break;
                        }
                }

                // 読み込んだデータを使って処理する部分
                // 例: 1画素目の値を表示
                if (bitsPerSample == 8) {
                        uint8_t* p = (uint8_t*)raster;
                        printf("First pixel values: ");
                        for (int c = 0; c < samplesPerPixel; c++) printf("%u ", p[c]);
                        printf("\n");
                }
                _TIFFfree(raster);
                TIFFClose(tif);
        }
        else {
                fprintf(stderr, "Could not open TIFF file.\n");
        }


                std::transform(image_paths.begin() + z, image_paths.begin() + end, std::back_inserter(images), [](auto &f) { return cv::imread(f.string(), cv::IMREAD_UNCHANGED); });
                create_directory(tmpDir / std::to_string(z));
                //mi::repeat_mt([&counter, &images, &sx, &sy, &z, &get_tmp_filename, &params]() {
                        for (uint32_t y = counter.get(); y < sy; y = counter.get()) {
                                std::vector<cv::Mat> local_images;
                                std::transform(images.begin(), images.end(), std::back_inserter(local_images),[&y, &sx](auto &image) { return cv::Mat(image, cv::Rect(cv::Point(0, int(y)), cv::Size(int(sx), 1))); }); // cut
                                cv::Mat local;
                                cv::vconcat(local_images, local);
                                xyz2zxy::write_image(get_tmp_filename(y, z), local, params);
                        }
                //});
                xyz2zxy::progress_bar(mtx, z + uint32_t(images.size()), sz, step1Str);
                counter.reset(0);
        }
        std::cerr << std::endl;



        return 0;
}
