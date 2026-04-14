#include "FileLogger.h"

#include <ctime>

namespace Util
{
  //---------------------------------------------- CONSTRUCTOR -----------------------------------------------
  FileLogger::FileLogger(const std::string& fileName, Util::LogLevel::Enum logLevel, bool showTime)
  :Logger(logLevel)
  ,fileName(fileName + ".log")
  ,showTime(showTime)
  {
  }
  //------------------------------------------------- FLUSH --------------------------------------------------
  bool FileLogger::flush(const char* data)
  {
    FILE *f = fopen(fileName.c_str(),"at");
    if (!f)
      return false;
    if (showTime)
    {
      char timeBuffer[9] = {};
      const std::time_t now = std::time(nullptr);
      if (const std::tm* localTime = std::localtime(&now))
      {
        std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", localTime);
      }
      fprintf(f, "%s ", timeBuffer);
    }
    fprintf(f, "%s \n", data);
    fclose(f);
    return true;
  }
}
