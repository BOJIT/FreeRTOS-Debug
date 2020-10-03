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

/** @brief Task handle that can be passed to the ISR */
TaskHandle_t debug_task;

#if DEBUG_LEVEL >= DEBUG_ERRORS
    /**
     * @brief This message is added to the queue if
     * there is only one space left.
     */
    static debug_t queue_full;

    /** @brief The queue itself */
    static QueueHandle_t debug_queue;

    /**
     * @brief Function pointer for the debug hardware initialisation function.
     */
    static void (*global_init_func)(void);

    /**
     * @brief Function pointer for actual debug character write.
     * This is a global variable as a function pointer cannot easily
     * be passed to a task.
     */
    static void (*global_send_func)(char);

    /**
     * @brief Function pointer for the system reset function.
     */
    static void (*global_reset_func)(void);
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
                    debug.task_handle = debug_task;
                    char full_message[] = "Queue Full!\n";
                    queue_full.message = pvPortMalloc(sizeof(full_message));
                    xQueueSend(debug_queue, &queue_full, 0);
                    break;
                }
            default:
                debug.task_handle = xTaskGetCurrentTaskHandle();
                xQueueSend(debug_queue, &debug, 0);
                break;
        }
    }

    /**
     * @brief Sends a single character to the user-defined debug interface,
     * with direct task notification to prevent CPU time being used to wait.
     * @param character character to send to debug.
    */
    static void debug_send_char(char character) {
        global_send_func(character);
        //ulTaskNotifyTake(pdFALSE, portMAX_DELAY); // Wait for ISR

                /* Reset the task notifier for next ISR */
                //debug_task = xTaskGetCurrentTaskHandle();
    }

    /**
     * @brief Task that handles actually sending the messages in a multi-threaded
     * environment.
     * @param args unused.
     */
    static void debug_handler(void *args __attribute((unused)))
    {
        /*
         * Calling the initialisation function here ensures the scheduler is
         * running in case an interrupt fires immediately.
         */
        global_init_func();
        for(;;) {
            /* Block until there is an item in the queue */
            debug_t debug_next;
            xQueueReceive(debug_queue, &debug_next, portMAX_DELAY);
            /* Print debug type and calling task */
            debug_send_char(debug_next.type);
            debug_send_char(' ');
            debug_send_char('-');
            debug_send_char(' ');
            char* task_ptr = pcTaskGetName(debug_next.task_handle);
            while(*task_ptr != '\0') {
                /* Print Character to debug console */
                debug_send_char(*task_ptr);

                task_ptr++;
            }
            debug_send_char(' ');
            debug_send_char('-');
            debug_send_char(' ');

            /* Write out message */
            char* message_ptr = debug_next.message;
            while(*message_ptr != '\0') {
                /* Print Character to debug console */
                debug_send_char(*message_ptr);

                message_ptr++;
            }
            debug_send_char('\n');

            /* Free the memory allocated to the message string */
            vPortFree(message_ptr);
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
 * @param reset_func function pointer to system reset function.
 *
 * @retval pointer to handle of the task that was created to handle debugging.
 * This task will block after a character is sent, so should be unblocked with a
 * direct task notification in an ISR.
 */
TaskHandle_t* debugInitialise(size_t queue_length, void (*init_func)(void),
                            void (*send_func)(char), void (*reset_func)(void))
{
    #if DEBUG_LEVEL >= DEBUG_ERRORS
        /* Call passed initialisation function and assign the fptrs */
        global_init_func = init_func;
        global_send_func = send_func;
        global_reset_func = reset_func;

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
        global_reset_func();
    #endif /* DEBUG_LEVEL >= DEBUG_MINIMAL */
}
