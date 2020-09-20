/**
 * @file
 * @brief functions for FreeRTOS-Debug library
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 20 September 2020
 */

#include "FreeRTOS-Debug.h"

#include <stdio.h>

#include <libopencm3/cm3/nvic.h>

/*----------------------------- Global Variables -----------------------------*/

/** @brief This message is added to the queue if there is only one space left */
debug_t queue_full;

/*----------------------------- Private Functions ----------------------------*/

/** 
 * @brief Internal function used to allocate memory for the error string.
 * @param length length of message passed in printf-style message.
*/
char* debug_pre_message(size_t length)
{
    return NULL;
}

/** 
 * @brief Internal function used add debug message to the queue.
 * @param debug_type debug message type - see Debug Types.
 * @param length length of message passed in printf-style message.
*/
void debug_post_message(debug_t debug)
{

}

/**
 * @brief Task that handles actually sending the messages in a multi-threaded
 * environment.
 * @param send_func function pointer to a function that sends one char over
 * serial in a non-blocking manner.
*/
static void debug_handler(void *send_func)
{
    for(;;) {

    }
}

/*------------------------------ Public Functions ----------------------------*/

/** 
 * @brief Initialise the queues and tasks associated with the debug handler.
 * This function should be called after the scheduler has started.
 * @param send_func function pointer to a function that sends one char over
 * serial in a non-blocking manner.
 * 
 * @retval handle of the task that was created to handle the debug messages.
 * This task will block after a character is sent, so should be unblocked with a
 * direct task notification in an ISR.
*/
TaskHandle_t debugInitialise(void (*send_func)(char), void (*init_func)(void))
{
    /* Call passed initialisation function */
    (*init_func)();

    /* Create debug task and pass handle back to the user application */
    TaskHandle_t task_handle;
    xTaskCreate(debug_handler, "debug", 350, (void *)send_func, 1, &task_handle);
    return task_handle;
}

/** 
 * @brief Suspend all tasks and halt everything for debugging.
*/
void debugFreeze(void)
{
    vTaskSuspendAll();
}

/** 
 * @brief Suspend the calling task for debugging.
*/
void debugFreezeTask(void)
{
    vTaskSuspend((TaskHandle_t)NULL);
}

/** 
 * @brief Reset the ARM CPU.
*/
void debugReset(void)
{
    reset_handler();
}
