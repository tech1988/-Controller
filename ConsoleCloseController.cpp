#include "ConsoleCloseController.h"

#ifdef WIN32
#include <Windows.h>
#include <cstring>
#else
#include <csignal>
#endif

ConsoleCloseController ConsoleCloseController::сonsoleCloseController = ConsoleCloseController();

ConsoleCloseController::ConsoleCloseController(){}

#ifdef WIN32
int WINAPI ConsoleCloseController::WinCallback(unsigned long)
{
    std::lock_guard<std::mutex> lock(сonsoleCloseController.mutex);

    while(!сonsoleCloseController.stack.empty())
    {
          if(сonsoleCloseController.stack.top()) сonsoleCloseController.stack.top()();
          сonsoleCloseController.stack.pop();
    }

    Sleep(10000);

    return TRUE;
}
#else
static void ConsoleCloseController::SignalCallback(int)
{
    std::lock_guard<std::mutex> lock(сonsoleCloseController.mutex);

    while(!сonsoleCloseController.stack.empty())
    {
          if(сonsoleCloseController.stack.top()) сonsoleCloseController.stack.top()();
          сonsoleCloseController.stack.pop();
    }
}
#endif

bool ConsoleCloseController::initialization(const std::function<void (std::string_view)> & errorLogger)
{
    std::lock_guard<std::mutex> lock(сonsoleCloseController.mutex);

    if(сonsoleCloseController.init.load()) return false;

#ifdef WIN32
    if(!SetConsoleCtrlHandler(&сonsoleCloseController::WinCallback, true))
    {
       if(errorLogger)
       {
          LPSTR errorText = nullptr;

          FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_IGNORE_INSERTS,
                         nullptr,
                         GetLastError(),
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         reinterpret_cast<LPSTR>(&errorText),
                         0,
                         nullptr);

          if(errorText != nullptr)
          {
             errorLogger("Console control handler set error: " + std::string(errorText));
             LocalFree(errorText);
          }
       }

       return false;
    }
#else
    std::signal(SIGINT, SignalCallback);
    std::signal(SIGTERM, SignalCallback);
#endif
    сonsoleCloseController.init = true;
    return true;
}

bool ConsoleCloseController::isInitialized(){ return сonsoleCloseController.init; }

void ConsoleCloseController::addFunction(const std::function<void()> & func)
{
    std::lock_guard<std::mutex> lock(сonsoleCloseController.mutex);
    if(func) сonsoleCloseController.stack.push(func);
}
