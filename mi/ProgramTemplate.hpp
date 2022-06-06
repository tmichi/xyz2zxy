/**
 * @file ProgramTemplate.hpp
 * @brief
 * @author Takashi Michikawa <tmichi@me.com>
 * @copyright (c) 2017  Takashi Michikawa
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 */
#ifndef MI_PROGRAM_TEMPLATE_HPP
#define MI_PROGRAM_TEMPLATE_HPP

#include "Attribute.hpp"
#include "Argument.hpp"
#include <type_traits>

namespace mi {
        class ProgramTemplate {
        private:
                std::string cmdStr_;
                std::string version_;
                std::unique_ptr<AttributeSet> attr_;
                bool isDebugModeOn_;
                bool result_code_;
        public:
                ProgramTemplate() = delete;

                explicit ProgramTemplate(const mi::Argument &arg, [[maybe_unused]] const std::string &cmdStr = "a.out", const std::string version="1.0.0")
                        : cmdStr_(arg.get<std::string>(0)), version_(version), attr_(new AttributeSet()),
                          isDebugModeOn_(arg.exist("--debug")), result_code_(arg.exist("--version")) {
                        if (arg.exist("--version")) {
                                throw std::runtime_error(this->cmdStr_+" "+this->version_);
                        }
                }

                ProgramTemplate(const ProgramTemplate &) = delete;

                void operator=(const ProgramTemplate &) = delete;

                virtual ~ProgramTemplate() = default;

                [[deprecated("Use mi::ProgramTemplate::execute<YourProgram>(const int, const char**) function")]]
                static int execute(ProgramTemplate &cmd) {
                        return cmd.run() ? EXIT_SUCCESS : EXIT_FAILURE;
                }

                template<typename T>
                static auto execute(const int argc,
                    const char** argv) -> decltype(typename std::enable_if<std::is_base_of<ProgramTemplate, T>::value>::type(), int()) {
                    try {
                        const mi::Argument arg(argc, argv);
                        T program(arg);
                        if (program.isDebugMode()) {
                            std::cerr << "Debug mode." << std::endl;
                        }
                        if (!program.run()) {
                            std::cerr << " Computation failed" << std::endl;
                        }
                    }
                    catch (std::exception& e) {
                        std::cerr << e.what() << std::endl;
                    }
                    catch (...){
                        std::cerr << "Unknown error detected." << std::endl;
                    }
                
                        return 0;
                }

                template<typename T>
                static auto execute(const int argc,
                                    const char **argv) -> decltype(typename std::enable_if<!std::is_base_of<ProgramTemplate, T>::value>::type(), int()) {
                        std::cerr << argc << " " << argv[0] << std::endl;
                        std::cerr
                                << "invalid call of ProgramTemplate::execute<T>(). T must be a subclass of mi::ProgramTemplate."
                                << std::endl;
                        return -1;
                }

        protected:
                virtual bool run() {
                        return this->result_code_;
                }

                AttributeSet &getAttributeSet() {
                        return *(this->attr_);
                }

                bool isDebugMode() const {
                        return this->isDebugModeOn_;
                }

                void set_failed() {
                        this->result_code_ = false;
                }
        };//class ProgramTemplate
}
#endif //MI_PROGRAM_TEMPLATE_HPP



