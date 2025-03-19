#ifndef CONSOLE_HPP
#define CONSOLE_HPP

class Console 
{
    public:
        Console() {};
        ~Console() {};

        void Run();

    private:
        static constexpr int MAX_CMD_LEN = 255;
        char buf[MAX_CMD_LEN + 1];

        void readLine();

        Console (const Console&) = delete;
        Console& operator= (const Console&) = delete;

        static const char * const TAG;
};

#endif // CONSOLE_HPP