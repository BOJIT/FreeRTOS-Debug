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
    debug_t queue_full;

    /** @brief The queue itself */
    QueueHandle_t debug_queue;

    /** @brief Task handle that can be passed to the ISR */
    TaskHandle_t debug_task;

    /**
     * @brief Function pointer for actual debug character write.
     * This is a global variable as a function pointer cannot easily
     * be passed to a task.
     */
    void (*debug_func)(char);
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
     * @param args unused.
     */
    static void debug_handler(void *args __attribute((unused)))
    {
        for(;;) {
            /* Block until there is an item in the queue */
            debug_t debug_next;
            xQueueReceive(debug_queue, &debug_next, portMAX_DELAY);
            for(int i = 0; i < 5; i++) {
                ulTaskNotifyTake(pdFALSE, portMAX_DELAY); // Wait for ISR

                /* Reset the task notifier for next ISR */
                debug_task = xTaskGetCurrentTaskHandle();

                /* Print Character to UART */
                debug_func(65);
            }
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
 * @retval pointer to handle of the task that was created to handle debugging.
 * This task will block after a character is sent, so should be unblocked with a
 * direct task notification in an ISR.
 */
TaskHandle_t* debugInitialise(size_t queue_length, void (*init_func)(void),
                                                    void (*send_func)(char))
{
    #if DEBUG_LEVEL >= DEBUG_ERRORS
        /* Call passed initialisation function */
        (*init_func)();
        debug_func = send_func;

        /* Initialise message queue */
        debug_queue = xQueueCreate(queue_length, sizeof(debug_t));

        /* Create debug task and pass handle back to the user application */
        xTaskCreate(debug_handler, "debug", 350, NULL, 1, &debug_task);

        /* Populate 'Queue Full' message partially */
        queue_full.type = DEBUG_TYPE_ERROR;
        queue_full.task_handle = debug_task;
    #endif /* DEBUG_LEVEL >= DEBUG_ERRORS */
    return &debug_task;
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
