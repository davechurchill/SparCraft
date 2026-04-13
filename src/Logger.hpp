#pragma once

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace SparCraft
{

inline std::string CurrentDateTime()
{
    const auto now = std::chrono::system_clock::now();
    const auto nowTime = std::chrono::system_clock::to_time_t(now);
    const auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm localTime {};
#if defined(_MSC_VER)
    localtime_s(&localTime, &nowTime);
#else
    localtime_r(&nowTime, &localTime);
#endif

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << milliseconds.count();
    return oss.str();
}

inline std::string CurrentTime()
{
    const auto now = std::chrono::system_clock::now();
    const auto nowTime = std::chrono::system_clock::to_time_t(now);
    const auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm localTime {};
#if defined(_MSC_VER)
    localtime_s(&localTime, &nowTime);
#else
    localtime_r(&nowTime, &localTime);
#endif

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << milliseconds.count();
    return oss.str();
}

inline std::string BuildDateTime()
{
    const char * compileDate = __DATE__;
    const char * compileTime = __TIME__;

    std::tm tm {};
    std::istringstream dateStream(compileDate);
    dateStream >> std::get_time(&tm, "%b %d %Y");
    std::istringstream timeStream(compileTime);
    timeStream >> std::get_time(&tm, "%H:%M:%S");

    std::ostringstream formatted;
    formatted << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return formatted.str();
}

inline std::string ReplaceNewlines(const std::string & input)
{
    std::stringstream ss;

    for (char c : input)
    {
        ss << c;
        if (c == '\n')
        {
            ss << '[' << CurrentTime() << "] ";
        }
    }

    return ss.str();
}

class Logger
{
    bool            m_printToConsole = false;
    std::string     m_logFileName = "sparcraft_error_log.txt";
    std::ofstream   m_fout;

    Logger() = default;
    ~Logger() = default;

    void EnsureOpen()
    {
        if (m_fout.is_open())
        {
            return;
        }

        // Open in append mode to preserve existing logs.
        m_fout.open(m_logFileName, std::ios::out | std::ios::app | std::ios::binary);
        if (!m_fout.is_open())
        {
            std::cerr << "Failed to open log file: " << m_logFileName << '\n';
        }
    }

public:
    static Logger & Instance()
    {
        static Logger instance;
        instance.EnsureOpen();
        return instance;
    }

    Logger(const Logger &) = delete;
    Logger & operator=(const Logger &) = delete;

    void SetLogFileName(const std::string & fileName)
    {
        m_logFileName = fileName;
        if (m_fout.is_open())
        {
            m_fout.close();
        }
        EnsureOpen();
    }

    void ClearLogFile()
    {
        if (m_fout.is_open())
        {
            m_fout.close();
        }

        std::ofstream truncate(m_logFileName, std::ios::out | std::ios::trunc | std::ios::binary);
        truncate.close();
        EnsureOpen();
    }

    void WriteSystemInfo()
    {
        Instance() << "[" << CurrentTime() << "] ";
        Instance() << "Build Date: " << BuildDateTime() << "\n";
        Instance() << "Run   Date: " << CurrentDateTime() << "\n\n";
    }

    void WriteToLog(const std::string & message)
    {
        const std::string output = ReplaceNewlines(message);

        try
        {
            EnsureOpen();
            if (!m_fout.is_open())
            {
                throw std::ios_base::failure("Log file is not open.");
            }

            m_fout << output;
            m_fout.flush();

            if (!m_fout.good())
            {
                throw std::ios_base::failure("Error occurred while writing to the log file.");
            }

            if (m_printToConsole)
            {
                std::cout << output;
            }
        }
        catch (const std::ios_base::failure & e)
        {
            std::cerr << "Logging error: " << e.what() << '\n';
        }
        catch (const std::exception & e)
        {
            std::cerr << "Unexpected logging error: " << e.what() << '\n';
        }
        catch (...)
        {
            std::cerr << "Unknown logging error occurred.\n";
        }
    }

    // Compatibility wrappers for existing older API.
    void log(const std::string & logFile, std::string & msg)
    {
        SetLogFileName(logFile);
        WriteToLog(msg);
    }

    void clearLogFile(const std::string & logFile)
    {
        SetLogFileName(logFile);
        ClearLogFile();
    }

    void SetPrintToConsole(const bool printToConsole)
    {
        m_printToConsole = printToConsole;
    }

    Logger & operator<<(const char * message)
    {
        WriteToLog(std::string(message ? message : ""));
        return *this;
    }

    Logger & operator<<(const std::string & message)
    {
        WriteToLog(message);
        return *this;
    }

    template<typename T>
    Logger & operator<<(const T & message)
    {
        std::ostringstream oss;
        oss << message;
        WriteToLog(oss.str());
        return *this;
    }
};

inline Logger & SCLog()
{
    return Logger::Instance();
}

}
