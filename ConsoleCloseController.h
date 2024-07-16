#ifndef CONSOLECLOSECONTROLLER_H
#define CONSOLECLOSECONTROLLER_H

#include <atomic>
#include <mutex>
#include <stack>
#include <functional>

class ConsoleCloseController final
{
   std::atomic_bool init = false;
   std::mutex mutex;
   std::stack<std::function<void()>> stack;
   static ConsoleCloseController сonsoleCloseController;

   explicit ConsoleCloseController();

#ifdef WIN32
   static int __stdcall WinCallback(unsigned long);
#else
   static void SignalCallback(int);
#endif

public:
   static bool initialization(const std::function<void(std::string_view)> & errorLogger = nullptr);
   static bool isInitialized();
   static void addFunction(const std::function<void()> & func);
};

#endif // CONSOLECLOSECONTROLLER_H
