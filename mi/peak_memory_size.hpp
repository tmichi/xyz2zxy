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
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
//ref : https://msdn.microsoft.com/ja-jp/library/windows/desktop/ms682050(v=vs.85).aspx
#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <process.h>
#include <intrin.h>
#include <Psapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "psapi.lib")
#elif defined (__APPLE__)

#include <sys/resource.h>

#else
#endif
namespace mi {
        /**
         * @brief Return peak memory size in bytes.
         * @return Peak memory size.
         */
        inline size_t peak_memory_size() {
#if defined (__APPLE__)
                if (rusage ru; getrusage(RUSAGE_SELF, &ru) == 0) {
                        return size_t(ru.ru_maxrss);
                } else {
                        return 0;
                }
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
                PROCESS_MEMORY_COUNTERS pmc;
                HANDLE hProcess = OpenProcess ( PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId() );
                if ( GetProcessMemoryInfo ( hProcess, &pmc, sizeof ( pmc ) ) ) {
                        CloseHandle ( hProcess );
                        return  pmc.PeakWorkingSetSize;
                } else {
                        CloseHandle ( hProcess );   
                        return 0;    
                }
#else
#endif // defined _APPLE_
        }// peak_memory_size
} //namespace 
#endif