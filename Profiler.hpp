//==============================================================================
// Lightweight profiler
// Quentin Quadrat <lecrapouille@gmail.com>
//==============================================================================
// This file is a modified version of https://github.com/TheCherno/Hazel
// Hazel/master/Hazel/src/Hazel/Debug/Instrumentor.h
// Copyright TheCherno. License Apache-2.0
// For more details see https://www.youtube.com/watch?v=xlAH4dbMVnU
//==============================================================================

#ifndef TOOL_PROFILER_HPP
#  define TOOL_PROFILER_HPP

#  include <string>
#  include <chrono>
#  include <algorithm>
#  include <fstream>
#  include <thread>
#  include <iomanip>

namespace tool { namespace prof {

  // ***************************************************************************
  //! \class Class allowing to measure time of function. It generate a JSON file
  //! which can be loaded by the Google browser Chrome. For loading the
  //! generated file type in the url: "chrome://tracing/" (without '"' chars).
  //! \note Beware methods of this class are not thread safe!
  //! \note Do not use directly this class but use macros instead.
  // ***************************************************************************
  class Profiler
  {
  public:

    //--------------------------------------------------------------------------
    //! \brief Profiler can be access from anywhere thanks to the singleton.
    //--------------------------------------------------------------------------
    static Profiler& singleton()
    {
      static Profiler instance;
      return instance;
    }

    //--------------------------------------------------------------------------
    //! \brief Start recording a new session. The previous JSON file is erased.
    //! To make things simple, you cannot record several sessions at the same
    //! time with the same profiler.
    //--------------------------------------------------------------------------
    void begin(const char* name, const char* filepath = "profile.json")
    {
      m_started = true;
      m_file.open(filepath);
      m_file << std::setprecision(3) << std::fixed;
      header();
      m_session = name;
    }

    //--------------------------------------------------------------------------
    //! \brief End recording the session.
    //--------------------------------------------------------------------------
    void end()
    {
      if (!m_started)
        return ;

      footer();
      m_started = false;
    }

    //--------------------------------------------------------------------------
    //! \brief Write time measurement in the json file.
    //--------------------------------------------------------------------------
    void write(const char* name, double const start, double const end, size_t const threadID)
    {
      if (!m_started)
        return ;

      m_file << "{\"cat\":\"function\",\"dur\":" << (end - start);
      m_file << ",\"name\":\"" << name;
      m_file << "\",\"ph\":\"X\",\"pid\":0,\"tid\":" << threadID;
      m_file << ",\"ts\":" << start;
      m_file << "},\n";
    }

  private:

    void header()
    {
      m_file << "{\"otherData\": {},\"traceEvents\":[\n";
    }

    void footer()
    {
      m_file << "{} ]}" << std::endl;
    }

    Profiler()
    {}

  private:

    //! \brief session name.
    std::string m_session;
    //! \brief JSON file we are going to write on.
    std::ofstream m_file;
    //! \brief Allow to write in JSON file only when a session has started.
    bool m_started = false;
  };

  // ***************************************************************************
  //! \class Proxy class measuring the elapsed time.
  // ***************************************************************************
  class InstrumentationTimer
  {
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

  public:

    //--------------------------------------------------------------------------
    //! \brief Start measuring the time.
    //! \param name the name of the function or the name of the scope.
    //--------------------------------------------------------------------------
    InstrumentationTimer(const char* name)
      : m_name(name)
    {
      m_startTimepoint = Clock::now();
    }

    //--------------------------------------------------------------------------
    //! \brief Stop measuring the time.
    //--------------------------------------------------------------------------
    ~InstrumentationTimer()
    {
      stop();
    }

  private:

    //--------------------------------------------------------------------------
    //! \brief Stop measuring the time and record the new measurement.
    //--------------------------------------------------------------------------
    void stop()
    {
      using namespace std::chrono;

      Profiler::singleton().write(
        m_name,
        static_cast<double>(time_point_cast<nanoseconds>(m_startTimepoint).time_since_epoch().count()) / 1000.0,
        static_cast<double>(time_point_cast<nanoseconds>(Clock::now()).time_since_epoch().count()) / 1000.0,
        std::hash<std::thread::id>{}(std::this_thread::get_id())
      );
    }

  private:

    //! \brief for computing the elapsed time
    TimePoint m_startTimepoint;
    //! \brief the name of the function or the name of the scope.
    const char* m_name;
  };
} } // namespace tool::prof

// *****************************************************************************
// Resolve which function signature macro will be used.
// *****************************************************************************
#  if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#    define FUNCTION_NAME __PRETTY_FUNCTION__
#  elif defined(__DMC__) && (__DMC__ >= 0x810)
#    define FUNCTION_NAME __PRETTY_FUNCTION__
#  elif defined(__FUNCSIG__)
#    define FUNCTION_NAME __FUNCSIG__
#  elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#    define FUNCTION_NAME __FUNCTION__
#  elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#    define FUNCTION_NAME __FUNC__
#  elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#    define FUNCTION_NAME __func__
#  elif defined(__cplusplus) && (__cplusplus >= 201103)
#    define FUNCTION_NAME __func__
#  else
#    define FUNCTION_NAME "FUNCTION_NAME unknown!"
#  endif

// *****************************************************************************
// Use these macros instead of accessing directly to methods of the class.
// *****************************************************************************
//! \brief Start a new profiling session. Beware the file is erased when opened.
//! \param name the name of the session.
//! \param filepath the path of the json file to store data.
#  define START_PROFILING(name, filepath) ::tool::prof::Profiler::singleton().begin(name, filepath)
//! \brief Stop profiling the current session.
#  define STOP_PROFILING() ::tool::prof::Profiler::singleton().end()
//! \brief Measure a part of code source. Add '{' and '}' to scope your code.
//! \param name the name of the scope.
#  define PROFILE_SCOPE(name) ::tool::prof::InstrumentationTimer timer##__LINE__(name);
//! \brief Measure a whole function.
#  define PROFILE_FUNCTION() PROFILE_SCOPE(FUNCTION_NAME)

#endif // TOOL_PROFILER_HPP
