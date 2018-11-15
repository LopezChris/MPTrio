#ifndef EXAMPLES_HPP_
#define EXAMPLES_HPP_

#include "cpp_task.hpp"
#include "FreeRTOS.h"
#include "semphr.h"



/**
 * This is the simplest example of a task.  It just computes some work
 * periodically and prints status out.
 */
class example_task : public scheduler_task
{
    public:
        example_task();
        bool run(void *p);
};



/**
 * This example shows how to use FreeRTOS based alarms such that you can design
 * tasks that do work every second, every minute etc.
 */
class example_alarm : public scheduler_task
{
    public:
        example_alarm();
        bool init(void);        ///< We create our semaphores heres. No FreeRTOS Blocking API here!!!
        bool taskEntry(void);   ///< We enable the alarms here.
        bool run(void *p);      ///< We run our code here

    private:
        xSemaphoreHandle mAlarmSec;
        xSemaphoreHandle mAlarmMin;
};



/**
 * This example shows how to log information and use Queue Set
 */
class example_logger_qset : public scheduler_task
{
    public :
        example_logger_qset();
        bool init(void);
        bool run(void *p);

    private :
        xSemaphoreHandle mSec, mMin;
};


/**
 * queue_tx and queue_rx classes show how two tasks can communicate with each other
 * using queue.
 */
class queue_tx : public scheduler_task
{
    public :
        queue_tx();
        bool init(void);
        bool run(void *p);
};

class queue_rx : public scheduler_task
{
    public :
        queue_rx();
        bool run(void *p);
};



#endif /* EXAMPLES_HPP_ */
