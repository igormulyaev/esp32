#include "Task.hpp"

// -----------------------------------------------------------------------
void Task :: taskFunction (void* taskObject) 
{
    static_cast<Task*>(taskObject) -> execute();
    vTaskDelete (NULL);
    static_cast<Task*>(taskObject) -> taskHandle = NULL;
}

// -----------------------------------------------------------------------
void Task ::  deleteTask()
{
    if (taskHandle != NULL) 
    {
        vTaskDelete(taskHandle);
        taskHandle = NULL;
    }
}

// -----------------------------------------------------------------------
BaseType_t Task :: createTask (
    const char * const name
    , const uint16_t stackSize
    , const UBaseType_t priority
    , const BaseType_t xCoreID
) 
{
    if (taskHandle == NULL) 
    {
        return xTaskCreatePinnedToCore (
            taskFunction
            , name
            , stackSize
            , this
            , priority
            , &taskHandle
            , xCoreID
        );
    }
    return pdFAIL;
}