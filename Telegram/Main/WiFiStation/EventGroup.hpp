#ifndef EVENTGROUP_HPP
#define EVENTGROUP_HPP

#include "freertos/event_groups.h"

class EventGroup
{
    public:
        EventGroup() {};
        ~EventGroup() {};

        inline void Init() { handle = xEventGroupCreateStatic (&storage); };

        inline void SetBits (const EventBits_t bits) { xEventGroupSetBits (handle, bits); };

        inline EventBits_t Wait (const EventBits_t waitFor) 
        { 
            return xEventGroupWaitBits (handle, waitFor, pdTRUE, pdFALSE, portMAX_DELAY); 
        };

    private:
        StaticEventGroup_t storage;
        EventGroupHandle_t handle;
};

#endif
