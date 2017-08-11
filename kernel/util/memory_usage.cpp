#include <kernel/util/memory_usage.hpp>
#include <kernel/util/exception.hpp>
#include <kernel/util/os_windows.hpp>

#ifdef __unix__
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#endif

#include <fstream>
#include <vector>
#include <string>

namespace FEAT
{
  namespace Util
  {
    MemUsageInfo get_memory_usage()
    {
      MemUsageInfo info;

      /// \platformswitch linux, windows and bsd all have different memory statistic tools
#if defined(__linux)
      String line;
      std::ifstream status_file("/proc/self/status");
      if (!status_file.is_open())
        throw InternalError(__func__, __FILE__, __LINE__, "could not open /proc/self/status!");

      /* man 5 proc:
       *   /proc/[pid]/status
       *   Provides much of the information in /proc/[pid]/stat and
       *   /proc/[pid]/statm in a format that's easier for humans to parse.
       *   The fields are as follows:
       *     Name: Command run by this process.
       *     State: Current state of the process.  One of "R (running)", "S (sleeping)", "D (disk sleep)", "T (stopped)", "T  (trac-
       *     ing stop)", "Z (zombie)", or "X (dead)".
       *     Tgid: Thread group ID (i.e., Process ID).
       *     Pid: Thread ID (see gettid(2)).
       *     PPid: PID of parent process.
       *     TracerPid: PID of process tracing this process (0 if not being traced).
       *     Uid, Gid: Real, effective, saved set, and file system UIDs (GIDs).
       *     FDSize: Number of file descriptor slots currently allocated.
       *     Groups: Supplementary group list.
       *!    VmPeak: Peak virtual memory size.
       *!    VmSize: Virtual memory size.
       *     VmLck: Locked memory size (see mlock(3)).
       *!    VmHWM: Peak resident set size ("high water mark").
       *!    VmRSS: Resident set size.
       *     VmData, VmStk, VmExe: Size of data, stack, and text segments.
       *     VmLib: Shared library code size.
       *     VmPTE: Page table entries size (since Linux 2.6.10).
       *     Threads: Number of threads in process containing this thread.
       *     [...]
       */
      while (std::getline(status_file, line))
      {
        if (line.starts_with("VmRSS"))
        {
          std::vector<String> v;
          line.split_by_charset(v);
          if (v.at(v.size()-1) != "kB")
            throw InternalError(__func__, __FILE__, __LINE__, "get_memory_usage: unit mismatch!");
          info.current_physical = std::stoul(v.at(v.size()-2));
          info.current_physical *= 1024;
          continue;
        }

        if (line.starts_with("VmHWM"))
        {
          std::vector<String> v;
          line.split_by_charset(v);
          if (v.at(v.size()-1) != "kB")
            throw InternalError(__func__, __FILE__, __LINE__, "get_memory_usage: unit mismatch!");
          info.peak_physical = std::stoul(v.at(v.size()-2));
          info.peak_physical *= 1024;
          continue;
        }

        if (line.starts_with("VmSize"))
        {
          std::vector<String> v;
          line.split_by_charset(v);
          if (v.at(v.size()-1) != "kB")
            throw InternalError(__func__, __FILE__, __LINE__, "get_memory_usage: unit mismatch!");
          info.current_virtual = std::stoul(v.at(v.size()-2));
          info.current_virtual *= 1024;
          continue;
        }

        if (line.starts_with("VmPeak"))
        {
          std::vector<String> v;
          line.split_by_charset(v);
          if (v.at(v.size()-1) != "kB")
            throw InternalError(__func__, __FILE__, __LINE__, "get_memory_usage: unit mismatch!");
          info.peak_virtual = std::stoul(v.at(v.size()-2));
          info.peak_virtual *= 1024;
          continue;
        }

        if (line.starts_with("VmSwap"))
        {
          std::vector<String> v;
          line.split_by_charset(v);
          if (v.at(v.size()-1) != "kB")
            throw InternalError(__func__, __FILE__, __LINE__, "get_memory_usage: unit mismatch!");
          info.current_swap = std::stoul(v.at(v.size()-2));
          info.current_swap *= 1024;
          continue;
        }
      }
      status_file.close();

#elif defined(_WIN32)

    unsigned long long work_set_size(0ull), work_set_size_peak(0ull), page_file_usage(0ull), page_file_usage_peak(0ull);

    Windows::query_memory_usage(work_set_size, work_set_size_peak, page_file_usage, page_file_usage_peak);

    info.current_physical = std::size_t(work_set_size);
    info.current_virtual  = std::size_t(page_file_usage);
    info.peak_physical    = std::size_t(work_set_size_peak);
    info.peak_virtual     = std::size_t(page_file_usage_peak);
    info.current_swap     = std::size_t(0);

#elif defined(__unix__)
    // see:
    // https://linux.die.net/man/2/getrusage
    // https://www.freebsd.org/cgi/man.cgi?query=getrusage
    struct rusage r_usage;
    if (0 != getrusage(RUSAGE_SELF, &r_usage))
      throw InternalError(__func__, __FILE__, __LINE__, "Error in getrusage call!");
    info.peak_physical = (size_t)r_usage.ru_maxrss;
    info.peak_virtual = (size_t)r_usage.ru_maxrss;
    info.current_physical = (size_t)r_usage.ru_maxrss;
    info.current_virtual = (size_t)r_usage.ru_maxrss;
    info.current_swap = std::size_t(0);
    // 'ru_maxrss' is given in kilobytes
    info.peak_physical *= 1024u;
    info.peak_virtual *= 1024u;
    info.current_physical *= 1024u;
    info.current_virtual *= 1024u;
#endif

      return info;
    }

    String get_formatted_memory_usage()
    {
      String r;
      auto m = get_memory_usage();
      r += String("Current real:").pad_back(20) + stringify(m.current_physical / 1024 / 1024) + " MByte\n";
      r += String("Peak real:").pad_back(20) + stringify(m.peak_physical / 1024 / 1024) + " MByte\n";
      r += String("Current virtual:").pad_back(20) + stringify(m.current_virtual / 1024 / 1024) + " MByte\n";
      r += String("Peak virtual:").pad_back(20) + stringify(m.peak_virtual / 1024 / 1024) + " MByte\n";
      r += String("Current swap:").pad_back(20) + stringify(m.current_swap / 1024 / 1024) + " MByte\n";

      return r;
    }
  } // namespace Util
} // namespace FEAT
