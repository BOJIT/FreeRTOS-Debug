/**
 * @file
 * @brief functions for FreeRTOS-Debug library
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 20 September 2020
 */

#include "FreeRTOS-Debug.h"

#include <libopencm3/cm3/nvic.h>

#include "queue.h"

/*----------------------------- Global Variables -----------------------------*/

#if DEBUG_LEVEL >= DEBUG_ERRORS
    /**
     * @brief This message is added to the queue if
     * there is only one space left.
     */
    static debug_t queue_full;

    /** @brief The queue itself */
    static QueueHandle_t debug_queue;
#endif /* DEBUG_LEVEL >= DEBUG_ERRORS */

/*----------------------------- Private Functions ----------------------------*/

#if DEBUG_LEVEL >= DEBUG_ERRORS
    /**
     * @brief Internal function used add debug message to the queue.
     * @param debug_type debug message type - see Debug Types.
     * @param length length of message passed in printf-style message.
     */
    void debug_send_message(debug_t debug)
    {
        switch(uxQueueSpacesAvailable(debug_queue)) {
            case 0:
                break;
            case 1:
                {
                    char full_message[] = "Queue Full!\n";
                    queue_full.message = pvPortMalloc(sizeof(full_message));
                    xQueueSend(debug_queue, &queue_full, 0);
                    break;
                }
            default:
                xQueueSend(debug_queue, &debug, 0);
                break;
        }
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
#endif /* DEBUG_LEVEL >= DEBUG_ERRORS */

/*------------------------------ Public Functions ----------------------------*/

/**
 * @brief Initialise the queues and tasks associated with the debug handler.
 * @param queue_length defines the length of the message queue. While the a long
 * message queue does not use up much memory, the dynamically allocated message
 * strings that the queue items point to do. Keep as low as possible.
 * @param init_func function pointer to a function that initialises the output
 * means that send_func uses to do the logging.
 * @param send_func function pointer to a function that sends one char in a
 * non-blocking manner.
 * 
 * @retval handle of the task that was created to handle the debug messages.
 * This task will block after a character is sent, so should be unblocked with a
 * direct task notification in an ISR.
 */
TaskHandle_t debugInitialise(size_t queue_length, void (*init_func)(void),
                                                    void (*send_func)(char))
{
    TaskHandle_t task_handle;
    #if DEBUG_LEVEL >= DEBUG_ERRORS
        /* Call passed initialisation function */
        (*init_func)();

        /* Initialise message queue */
        debug_queue = xQueueCreate(queue_length, sizeof(debug_t));

        /* Create debug task and pass handle back to the user application */
        xTaskCreate(debug_handler, "debug", 350, (void *)send_func,
                                                            1, &task_handle);

        /* Populate 'Queue Full' message partially */
        queue_full.type = DEBUG_TYPE_ERROR;
        queue_full.task_handle = task_handle;
    #endif /* DEBUG_LEVEL >= DEBUG_ERRORS */
    return task_handle;
}

/**
 * @brief Suspend all tasks and halt everything for debugging.
 */
void debugFreeze(void)
{
    #if DEBUG_LEVEL >= DEBUG_FULL
        vTaskSuspendAll();
    #endif /* DEBUG_LEVEL >= DEBUG_FULL */
}

/**
 * @brief Suspend the calling task for debugging.
 */
void debugFreezeTask(void)
{
    #if DEBUG_LEVEL >= DEBUG_ERRORS
        vTaskSuspend((TaskHandle_t)NULL);
    #endif /* DEBUG_LEVEL >= DEBUG_ERRORS */
}

/**
 * @brief Reset the ARM CPU.
 */
void debugReset(void)
{
    #if DEBUG_LEVEL >= DEBUG_MINIMAL
        reset_handler();
    #endif /* DEBUG_LEVEL >= DEBUG_MINIMAL */
}
