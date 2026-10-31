// Regression coverage for buffer reuse, partial batches and multipage TIFF input.
#include <xyz2zxy.hpp>

namespace fs = std::filesystem;

void require(bool condition, const std::string &message) {
        if (!condition) throw std::runtime_error(message);
}

cv::Mat sample(int type, int z) {
        cv::Mat image(5, 7, type);
        for (int y = 0; y < image.rows; ++y) {
                for (int x = 0; x < image.cols; ++x) {
                        for (int c = 0; c < image.channels(); ++c) {
                                int value = 1000 * z + 100 * y + 10 * x + c;
                                int index = x * image.channels() + c;
                                if (image.depth() == CV_8U) image.ptr<uchar>(y)[index] = value % 251;
                                else image.ptr<uint16_t>(y)[index] = value;
                        }
                }
        }
        return image;
}

int image_type(const std::string &name) {
        const int depth = name.substr(0, 2) == "16" ? CV_16U : CV_8U;
        return CV_MAKETYPE(depth, name.back() - '0');
}

void prepare(const fs::path &root) {
        fs::remove_all(root);
        for (const std::string name : {"8UC1", "8UC3", "8UC4", "16UC1", "16UC3", "16UC4"}) {
                const int type = image_type(name);
                std::vector<cv::Mat> pages;
                for (int z = 0; z < 8; ++z) pages.push_back(sample(type, z));
                for (const std::string ext : {".png", ".tif"}) {
                        auto dir = root / (name + ext);
                        fs::create_directories(dir);
                        cv::Mat buffer;
                        std::vector<uchar> encoded;
                        uchar *pixels = nullptr;
                        for (int z = 0; z < 8; ++z) {
                                auto path = dir / std::format("image-{:05d}{}", z, ext);
                                require(cv::imwrite(path.string(), pages[z]), "write fixture");
                                xyz2zxy::read_image(path.string(), buffer, encoded, cv::IMREAD_UNCHANGED);
                                if (pixels) require(buffer.data == pixels, "decoded buffer was reallocated");
                                pixels = buffer.data;
                                require(cv::norm(buffer, pages[z], cv::NORM_INF) == 0, "decoded pixels changed");
                        }
                }
                auto mtif = root / (name + ".tifstack");
                fs::create_directories(mtif);
                require(cv::imwritemulti((mtif / "input.tif").string(), pages), "write TIFF stack");
                auto extracted = xyz2zxy::list_files(mtif / "input.tif", mtif / "tmp", ".tif");
                require(extracted.size() == pages.size(), "TIFF page count");
                for (size_t z = 0; z < pages.size(); ++z) {
                        auto actual = cv::imread(extracted[z].string(), cv::IMREAD_UNCHANGED);
                        require(cv::norm(actual, pages[z], cv::NORM_INF) == 0, "TIFF page repeated or changed");
                }
                fs::remove_all(mtif / "tmp");
        }
        // Failed reads must not silently reuse stale pixels.
        cv::Mat buffer;
        std::vector<uchar> encoded;
        bool failed = false;
        try { xyz2zxy::read_image((root / "missing.png").string(), buffer, encoded, cv::IMREAD_UNCHANGED); }
        catch (const std::runtime_error &) { failed = true; }
        require(failed, "missing image accepted");
}

void verify(const fs::path &root, int type, bool zxy) {
        const int planes = zxy ? 5 : 7;
        require(std::distance(fs::directory_iterator(root), fs::directory_iterator()) == planes, "output count");
        for (int plane = 0; plane < planes; ++plane) {
                auto path = root / std::format("image-{:05d}{}", plane, root.extension().string());
                auto actual = cv::imread(path.string(), cv::IMREAD_UNCHANGED);
                // The existing ANYDEPTH | ANYCOLOR intermediate read drops alpha.
                const int channels = std::min(CV_MAT_CN(type), 3);
                cv::Mat expected(zxy ? 7 : 8, zxy ? 8 : 5, CV_MAKETYPE(CV_MAT_DEPTH(type), channels));
                for (int z = 0; z < 8; ++z) {
                        auto source = sample(type, z);
                        for (int i = 0; i < (zxy ? 7 : 5); ++i) {
                                auto src = source.ptr(zxy ? plane : i) + (zxy ? i : plane) * source.elemSize();
                                auto dst = expected.ptr(zxy ? i : z) + (zxy ? z : i) * expected.elemSize();
                                std::copy_n(src, expected.elemSize(), dst);
                        }
                }
                require(actual.size() == expected.size() && actual.type() == expected.type(), "output shape/type");
                require(cv::norm(actual, expected, cv::NORM_INF) == 0, "output pixel mismatch");
        }
}

int main(int argc, char **argv) {
        try {
                require(argc >= 3, "arguments");
                if (std::string(argv[1]) == "prepare") prepare(argv[2]);
                else {
                        require(argc == 5, "verify arguments");
                        verify(argv[2], image_type(argv[3]), std::string(argv[4]) == "zxy");
                }
                return EXIT_SUCCESS;
        } catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
                return EXIT_FAILURE;
        }
}
