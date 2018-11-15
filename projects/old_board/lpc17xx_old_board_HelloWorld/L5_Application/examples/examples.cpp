#include <stdio.h>
#include "examples.hpp"
#include "rtc_alarm.h"
#include "rtc.h"
#include "file_logger.hpp"
#include "io.hpp"



example_task::example_task() :
    scheduler_task("ex_simple", 4*1024, PRIORITY_HIGH)
{
    /**
     * This is optional such that run() will be called every 3000ms.
     * If you don't do this, run() will be called without a delay.
     */
    setFrequency(3000);
}

bool example_task::run(void *p)
{
    printf("\n---------------------------------\n"
           "Status Report: \n");
    printf("Temperature: %i F\n", (int)TS.getFarenheit());
    printf("CPU Usage : %i %%\n", getCpuUsage());           /* get OUR tasks' cpu usage */
    printf("Free stack: %i bytes\n", (int)getFreeStack());  /* get number of bytes of free stack of our task */

    for(volatile int i=0; i<0xfffff; i++) {
        ; // Do dummy work
    }

    return true;
}






/**
 * This example shows how to setup alarms in the form of semaphore
 * that can be given every second, minute, hour etc.
 */
example_alarm::example_alarm() :
    scheduler_task("ex_alarm", 4*1024, PRIORITY_LOW),
    mAlarmSec(NULL),
    mAlarmMin(NULL)
{

}

bool example_alarm::init(void)
{
    vSemaphoreCreateBinary(mAlarmSec);
    vSemaphoreCreateBinary(mAlarmMin);

    return (mAlarmSec != NULL && mAlarmMin != NULL);
}

bool example_alarm::taskEntry(void)
{
    rtc_EnableRecurringAlarm(everySecond, &mAlarmSec);
    rtc_EnableRecurringAlarm(everyMinute, &mAlarmMin);

    return true;
}

bool example_alarm::run(void *p)
{
    static int print_first_sixty_secs = 0;
    ++print_first_sixty_secs;

    /**
     * Note that we use MAX delay while checking for mAlarmSec because if a second
     * hasn't elapsed, then by definition, minute and night won't elapse.
     */
    if (xSemaphoreTake(mAlarmSec, portMAX_DELAY)) {
        if (print_first_sixty_secs <= 60) {
            printf("tick %i -- ", rtc_gettime().sec);
        }
    }
    if (xSemaphoreTake(mAlarmMin, 0)) {
        puts("A minute has elapsed!");
    }

    return true;
}






/**
 * Queueset example shows how to use FreeRTOS to block on MULTIPLE semaphores or queues.
 * The idea is that we want to call our run() when EITHER of mSec, or mMin semaphore
 * is ready.
 * This example also shows how to log information to "log" file on flash memory.
 */
example_logger_qset::example_logger_qset() :
    scheduler_task("ex_log_qset", 2*1024, PRIORITY_LOW),
    mSec(NULL),
    mMin(NULL)
{
    vSemaphoreCreateBinary(mSec);
    vSemaphoreCreateBinary(mMin);
}

bool example_logger_qset::init(void)
{
    rtc_EnableRecurringAlarm(everySecond, &mSec);
    rtc_EnableRecurringAlarm(everyMinute, &mMin);

    /**
     * We want to block on both seconds and minutes, but any queues or semaphores
     * must be empty before adding to the queue set.  Binary sempaphore is not
     * empty after creating, so perform xSemaphoreTake()
     */
    xSemaphoreTake(mSec, 0);
    xSemaphoreTake(mMin, 0);
    initQueueSet(2, 2, mSec, mMin);

    /**
     * Queue Set will use this block time, and only call run() if either
     * mSec or mMin are given or when this timeout occurs.
     */
    setQueueSetBlockTime(portMAX_DELAY);

    return (mSec != NULL && mMin != NULL);
}

bool example_logger_qset::run(void *p)
{
    static int count = 0;

    if (getQueueSetSelection() == mSec) {
        // This must work but just in case, return "false" to stop this task.
        if(!xSemaphoreTake(mSec, 0)) {
            return false;
        }

        LOG_INFO_PRINTF("Example log");
    }
    else if(getQueueSetSelection() == mMin) {
        if(!xSemaphoreTake(mMin, 0)) {
            return false;
        }
        LOG_WARN_PRINTF("LOG WARNING: See me @ log file at flash memory by doing 'cat log' command");
    }
    else {
        LOG_ERROR("I should not get here!");
        return false;
    }

    /** When count exceeds 60, we stop this task on purpose */
    if (count++ < 60) {
        vTaskSuspend(getTaskHandle());
    }
    else {
        /**
         * Information is flushed automatically when file logger's buffer gets full,
         * but manually flush everything we've got.
         */
        FileLogger::getInstance().flush();
        puts("Information was logged to a file.  Type 'cat log' to see it.");
        puts("Suspending this task on purpose");
        return false;
    }

    return false;
}






/**
 * This example shows how two tasks can exchange information with each other using a queue.
 *      addSharedObject() adds a pointer to be shared, which we use to share queue handle.
 *      getSharedObject() gets the pointer which all tasks can obtain.
 */
queue_tx::queue_tx() :
    scheduler_task("ex_qtx", 1*1024, PRIORITY_LOW)
{
    /* Nothing to do */
}
bool queue_tx::init(void)
{
    // call our run() every second :
    setFrequency(1000);

    /**
     * Create a queue, and add to mShared object by the name of "my_queue"
     * One small error is not to check for NULL from xQueueCreate()
     */
    return addSharedObject("my_queue", xQueueCreate(1, sizeof(int)));
}
bool queue_tx::run(void *p)
{
    // Send only 10 samples, then suspend both tasks.
    static int sample = 0;
    if (++sample > 10) {
        // Get the pointer of the "ex_qrx" task
        scheduler_task *qrx_task = getTaskPtrByName("ex_qrx");

        // Suspend the other task and ourselves
        vTaskSuspend(qrx_task->getTaskHandle());
        vTaskSuspend(getTaskHandle());
    }

    /**
     * Send to "my_queue" our integer
     * One small error is that we don't check for NULL pointer from getSharedObject()
     */
    xQueueHandle queue_handle = getSharedObject("my_queue");
    return xQueueSend(queue_handle, &sample, 2000);
}

/**
 * Simple task that is receiving data from queue_tx class.
 * Since queue is already created by queue_tx::init(), we just use it in our run().
 */
queue_rx::queue_rx() :
    scheduler_task("ex_qrx", 2*1024, PRIORITY_LOW)
{

}
bool queue_rx::run(void *p)
{
    int sample = 0;
    xQueueHandle queue_handle = getSharedObject("my_queue");

    if (xQueueReceive(queue_handle, &sample, portMAX_DELAY)) {
        printf("Got %i from my_queue\n", sample);
    }
    return true;
}
