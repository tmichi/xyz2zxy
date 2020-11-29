#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

#include <filesystem>
namespace fs = std::filesystem;

namespace mi {
        std::vector<fs::path> list_files(const fs::path &path) {
                std::vector<fs::path> result;
                fs::directory_iterator end;
                for (fs::directory_iterator p(path); p != end; ++p) {
                        if (!fs::is_directory(p->path()) && p->path().filename().string().find_first_of(".") != 0 ) {
                                result.push_back(p->path());
                        }
                }
                return result;
        }
        
        bool make_directory(const fs::path &path) {
                try {
                        if (fs::exists(path)) {
                                if (!fs::is_directory(path)) {
                                        throw std::runtime_error( path.string() + " already exists, but is not directory.");
                                }
                        } else {
                                if (!fs::create_directory(path)) {
                                        throw std::runtime_error(path.string() + " cannot be created.");
                                }
                        }
                } catch (std::runtime_error& e ) {
                        std::cerr << e.what() << std::endl; // 失敗
                        return false;
                }
                return true;
        }
}