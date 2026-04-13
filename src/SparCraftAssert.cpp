#include "SparCraftAssert.h"
#include "SparCraftException.h"

#include <chrono>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <vector>

using namespace SparCraft;

namespace SparCraft
{
namespace Assert
{
    namespace
    {
        std::string FormatVariadicMessage(const char * msg, va_list args)
        {
            if (!msg)
            {
                return {};
            }

            va_list copy;
            va_copy(copy, args);
            const int requiredChars = std::vsnprintf(nullptr, 0, msg, copy);
            va_end(copy);

            if (requiredChars <= 0)
            {
                return std::string(msg);
            }

            std::vector<char> buffer(static_cast<size_t>(requiredChars) + 1, '\0');
            std::vsnprintf(buffer.data(), buffer.size(), msg, args);
            return std::string(buffer.data());
        }

        std::string BuildFailureMessage(
            const char * condition,
            const char * file,
            int line,
            const std::string & message)
        {
            std::stringstream ss;
            ss << '\n';
            ss << "!Assert:   " << (condition ? condition : "<unknown>") << '\n';
            ss << "File:      " << (file ? file : "<unknown>") << '\n';
            ss << "Message:   " << message << '\n';
            ss << "Line:      " << line << '\n';
            ss << "Time:      " << CurrentDateTime() << '\n';
            return ss.str();
        }
    }

    const std::string CurrentDateTime()
    {
        const auto now = std::chrono::system_clock::now();
        const auto nowTime = std::chrono::system_clock::to_time_t(now);
        const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::tm localTime {};
#if defined(_MSC_VER)
        localtime_s(&localTime, &nowTime);
#else
        localtime_r(&nowTime, &localTime);
#endif

        std::ostringstream oss;
        oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
        oss << '.' << std::setfill('0') << std::setw(3) << millis.count();

        return oss.str();
    }

    void ReportFailure(const GameState * state, const char * condition, const char * file, int line, const char * msg, ...)
    {
        va_list args;
        va_start(args, msg);
        const std::string formattedMessage = FormatVariadicMessage(msg, args);
        va_end(args);

        const std::string failureMessage = BuildFailureMessage(condition, file, line, formattedMessage);
        
        #if !defined(EMSCRIPTEN)
            std::cerr << "Assertion thrown!\n";
            std::cerr << failureMessage;
            throw SparCraftException(failureMessage, state);
        #else
            printf("C++ AI: AI Exception Thrown:\n %s\n", failureMessage.c_str());
            throw SparCraftException(failureMessage);
        #endif
    }
}
}

