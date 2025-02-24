#ifndef TASK_HPP
#define TASK_HPP

#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/task.h"

class Task 
{
    public:
        Task() {};
        virtual ~Task() { deleteTask(); };

        BaseType_t createTask (
            const char * const name
            , const uint16_t stackSize = 4096
            , const UBaseType_t priority = 1
            , const BaseType_t xCoreID = tskNO_AFFINITY
        );

        void deleteTask();

    private:
        TaskHandle_t taskHandle = NULL;

        virtual void execute() = 0;
        
        static void taskFunction (void* taskObject);
        
        Task (const Task&) = delete;
        Task& operator= (const Task&) = delete;
};
#endif // TASK_HPP