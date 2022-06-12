#include "Screen.h"

std::unique_ptr<WINDOW> Screen::logwindow = 0;
std::unique_ptr<WINDOW> Screen::inputwindow = 0;
bool Screen::Running = false;
std::queue<std::string> Screen::cmdBuffer = std::queue<std::string>();
std::thread Screen::inputThread = std::thread();

void Screen::Start(const bool logToFile)
{
    Screen::Running = true;
    //Screen::LogToFile = logToFile; //Why doesn't this compile?
    WINDOW* screen = initscr();
    raw();
    noecho();
    curs_set(0);
    resize_term(24, 80);
    start_color();
    halfdelay(1);
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    #ifdef __linux__
    #else
    resize_window(screen, 24, 80);
    #endif

    Screen::logwindow = std::unique_ptr<WINDOW>(newwin(23, 80, 0, 0));
    wrefresh(Screen::logwindow.get());
    scrollok(Screen::logwindow.get(), true);

    Screen::inputwindow = std::unique_ptr<WINDOW>(newwin(1, 80, 23, 0));
    attron(COLOR_PAIR(1));
    wrefresh(Screen::inputwindow.get());
    wclear(Screen::inputwindow.get());
    wbkgd(Screen::inputwindow.get(), COLOR_PAIR(1));
    attroff(COLOR_PAIR(1));

    wmove(Screen::inputwindow.get(), 0, 0);
    refresh();

    inputThread = std::thread(GetInput);
}

void Screen::GetInput()
{
    std::string input = std::string();
    while (Screen::Running)
    {
        wmove(Screen::inputwindow.get(), 0, 0);
        wrefresh(Screen::inputwindow.get());
        char c = getch();
        if (c == ERR)
            continue;

        switch (c)
        {
        #ifdef __linux__
        case 10:
        #else
        case 13:
        #endif
            wclear(Screen::inputwindow.get());
            cmdBuffer.push(std::string(input));
            input.resize(80);
            std::fill(input.begin(), input.begin() + 80, ' ');
            wprintw(Screen::inputwindow.get(), input.c_str(), c);
            wrefresh(Screen::inputwindow.get());
            input.clear();
            wprintw(Screen::inputwindow.get(), (input + std::string("_")).c_str(), c);
            break;
        case 8:
            if(input.size() > 0)
                input = input.substr(0, input.size() - 1);
            wclear(Screen::inputwindow.get());
            break;
        default:
            input += c;
            break;
        }
        wprintw(Screen::inputwindow.get(), (input + std::string("_")).c_str(), c);
    }
}

void Screen::Stop()
{
    Screen::Running = false;
    inputThread.join();
    endwin();
}

std::string Screen::PollCommand()
{
    if (Screen::cmdBuffer.empty())
        return std::string();
    std::string cmd = Screen::cmdBuffer.back();
    Screen::cmdBuffer.pop();
    return cmd;
}

void Screen::Log(const std::string& message, bool newline)
{
    //std::cout << message << std::endl;
    if(newline)
        waddstr(Screen::logwindow.get(), (message + "\n").c_str());
    else
        waddstr(Screen::logwindow.get(), (message).c_str());

    wrefresh(Screen::logwindow.get());
    wmove(Screen::inputwindow.get(), 0, 0);
    wrefresh(Screen::inputwindow.get());

    std::ofstream ofs("x3mp.log", std::ios::app | std::fstream::out);
    ofs << message << "\n";
    ofs.close();
}

void Screen::Log(const char* message, bool newline)
{
    Log(std::string(message), newline);
}

void Screen::LogError(const std::string& message, bool newline)
{
    Screen::Log("[ERR]" + message, newline);
}

void Screen::LogError(const char* message, bool newline)
{
    Screen::LogError(std::string(message), newline);
}

void Screen::LogDebug(const std::string& message, bool newline)
{
    Screen::Log("[DBG]" + message, newline);
}

void Screen::LogDebug(const char* message, bool newline)
{
    Screen::LogDebug(std::string(message), newline);
}