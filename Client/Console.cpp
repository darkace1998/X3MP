#include "console.h"


x3::Console::Console()
{
    //HANDLE _old_out = GetStdHandle(STD_OUTPUT_HANDLE);
    //HANDLE _old_err = GetStdHandle(STD_ERROR_HANDLE);
    //HANDLE _old_in = GetStdHandle(STD_INPUT_HANDLE);
    
    AllocConsole();
    
    //HANDLE _out = GetStdHandle(STD_OUTPUT_HANDLE);
    //HANDLE _err = GetStdHandle(STD_ERROR_HANDLE);
    //HANDLE _in = GetStdHandle(STD_INPUT_HANDLE);

    //SetConsoleMode(_out, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);
    //SetConsoleMode(_in, ENABLE_EXTENDED_FLAGS | ENABLE_QUICK_EDIT_MODE);
    freopen_s(&f, "CONOUT$", "w", stdout);
}

void x3::Console::Log(std::string str, MessageLevel lvl)
{
    std::string prefix;
    switch (lvl)
    {
    case MessageLevel::Debug:
        prefix = "[DBG]";
        break;
    case MessageLevel::Info:
        prefix = "[INF]";
        break;
    case MessageLevel::Error:
        prefix = "[ERR]";
        break;
    }

    std::cout << prefix << " " << str << std::endl;
}

void x3::Console::Log(const char* str, MessageLevel lvl)
{
    std::string prefix;
    switch (lvl)
    {
    case MessageLevel::Debug:
        prefix = "[DBG]";
        break;
    case MessageLevel::Info:
        prefix = "[INF]";
        break;
    case MessageLevel::Error:
        prefix = "[ERR]";
        break;
    }

    std::cout << prefix << " " << str << std::endl;
}

void x3::Console::Close()
{
    fclose(f);
    FreeConsole();
}
