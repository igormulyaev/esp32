#ifndef TELEGRAMTASK_HPP
#define TELEGRAMTASK_HPP

#include "System/Task.hpp"

class TelegramTask : public Task
{
    public:
        TelegramTask() {};
        virtual ~TelegramTask() {};

    private:
        void execute() override;

        TelegramTask (const TelegramTask&) = delete;
        TelegramTask& operator= (const TelegramTask&) = delete;

        static const char * const TAG;
};

#endif // TELEGRAMTASK_HPP