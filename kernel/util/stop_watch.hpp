#pragma once
#ifndef KERNEL_UTIL_STOP_WATCH_HPP
#define KERNEL_UTIL_STOP_WATCH_HPP 1

// includes, FEAST
#include <kernel/util/time_stamp.hpp>

namespace FEAST
{
  /**
   * \brief Stop-Watch class
   *
   * This class implements an incremental stop-watch.
   *
   * \author Peter Zajac
   */
  class StopWatch
  {
  private:
    TimeStamp _start_time;
    TimeStamp _stop_time;
    bool _running;
    long long _micros;

  public:
    StopWatch() :
      _running(false),
      _micros(0ll)
    {
    }

    /// Resets the elapsed time.
    void reset()
    {
      _running = false;
      _micros = 0ll;
    }

    /// Starts the stop-watch.
    void start()
    {
      if(!_running)
        _start_time.stamp();
      _running = true;
    }

    /// Stops the stop-watch and increments elapsed time.
    void stop()
    {
      if(_running)
      {
        _stop_time.stamp();

        // update elapsed time
        _micros += _stop_time.elapsed_micros(_start_time);
      }
      _running = false;
    }

    /// Returns \c true if the stop-watch is currently running.
    bool running() const
    {
      return _running;
    }

    /// Returns the total elapsed time in seconds.
    double elapsed() const
    {
      return 1E-6 * double(elapsed_micros());
    }

    /// Returns the total elapsed time in micro-seconds.
    long long elapsed_micros() const
    {
      if(!_running)
        return _micros;
      else
      {
        // stop-watch is currently running, so compute current time
        return _micros + TimeStamp().elapsed_micros(_start_time);
      }
    }

    /**
     * \brief Return the time elapsed in the stop-watch.
     *
     * See TimeStamp::format_micros() for more information about the formatting options.
     *
     * \param[in] bhours
     * Specifies whether hours are to be printed.
     *
     * \param[in] bmillis
     * Specifies whether milliseconds are to be printed.
     *
     * \returns
     * The time elapsed between in the stop-watch as a formatted string <code>h:mm:ss.nnn</code>.
     */
    String elapsed_string(bool bhours = true, bool bmillis = true) const
    {
      return TimeStamp::format_micros(elapsed_micros(), bhours, bmillis);
    }
  }; // class StopWatch
} // namespace FEAST

#endif // KERNEL_UTIL_STOP_WATCH_HPP
