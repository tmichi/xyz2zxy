/**
 * @file peak_memory_size.hpp
 * @brief
 * @author Takashi Michikawa <tmichi@me.com>
 * @copyright (c) 2018 -  Takashi Michikawa
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 */
#ifndef MI_PEAK_MEMORY_SIZE_HPP
#define MI_PEAK_MEMORY_SIZE_HPP 1
#if defined(_WIN32)
//ref : https://msdn.microsoft.com/ja-jp/library/windows/desktop/ms682050(v=vs.85).aspx
#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <process.h>
#include <intrin.h>
#include <Psapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "psapi.lib")
#elif defined (__APPLE__) || defined (__linux__) || defined (__cygwin__)
#include <sys/resource.h>
#else
//do something.
#endif
namespace mi {
        /**
         * @brief Return peak memory size in bytes.
         * @return Peak memory size.
         */
        inline size_t peak_memory_size() {
#if defined(_WIN32)
                PROCESS_MEMORY_COUNTERS pmc;
                HANDLE hProcess = OpenProcess ( PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId() );
                const size_t pms = GetProcessMemoryInfo ( hProcess, &pmc, sizeof ( pmc ) ? pms.PeakWorkingSetSize : 0 ;
                CloseHandle ( hProcess );
                return pms;
#elif defined (__APPLE__) || defined (__linux__) || defined (__cygwin__)
                #if defined (__APPLE__)
                const uint64_t scale = 1;
                #else// defined (__linux__) || defined (__cygwin__)
                const uint64_t scale = 1024;
                #endif
                struct rusage usage;
                getrusage(RUSAGE_SELF, &usage);
                return static_cast<size_t>(usage.ru_maxrss) * scale;
#else
                return 0;
#endif // __APPLE__
        }
} //namespace 
#endif