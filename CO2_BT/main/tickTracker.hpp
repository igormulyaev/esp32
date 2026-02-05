#ifndef TICK_TRACKER_HPP
#define TICK_TRACKER_HPP

class TickTracker
{
    public:
        TickTracker () : next(portMAX_DELAY), period(0) {}

        TickType_t getTimeToNextEvent() const 
        {
            TickType_t now = xTaskGetTickCount();
            if (next == portMAX_DELAY) {
                return portMAX_DELAY;
            }
            else if (next > now) {
                return next - now;
            }
            else {
                return 0;
            }
        }

        TickType_t getTimeToNextEvent(TickType_t timeToOther) const 
        {
            TickType_t timeToThis = getTimeToNextEvent();
            return (timeToThis < timeToOther) ? timeToThis : timeToOther;
        }

        void updateNext() 
        {
            if (period == 0) {
                next = portMAX_DELAY; // Disabled
            }
            else if (next != portMAX_DELAY) {
                next += period; // Regular update
            }
        }

        void setPeriod(TickType_t newPeriod) 
        {
            period = newPeriod;
            if (period == 0) {
                next = portMAX_DELAY; // Disabled
            }
            else if (next == portMAX_DELAY) {
                next = xTaskGetTickCount() + period; // Start counting from now
            }
        }

        bool isDue(TickType_t now) const 
        {
            return now >= next;
        }

    private:
        TickType_t next;
        TickType_t period;
};

#endif // TICK_TRACKER_HPP