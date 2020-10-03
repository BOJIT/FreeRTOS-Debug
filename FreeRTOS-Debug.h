/**
 * @file
 * @brief headers for FreeRTOS-Debug library
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 20 September 2020
 */

#ifndef __FREERTOS_DEBUG__
#define __FREERTOS_DEBUG__

#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"

/*------------------------- Public Macros and Structs ------------------------*/

/** @brief Debug Levels */
#define DEBUG_OFF       0
#define DEBUG_MINIMAL   1
#define DEBUG_ERRORS    2
#define DEBUG_WARNINGS  3
#define DEBUG_FULL      4

/** @brief Debug Types */
#define DEBUG_TYPE_INFO     'I'
#define DEBUG_TYPE_WARNING  'W'
#define DEBUG_TYPE_ERROR    'E'

/** @brief Parentheses removal macro */
#define ESC(...) __VA_ARGS__

/** @brief Debug struct that is added to the message queue */
typedef struct {
    char type;
    TaskHandle_t task_handle;
    char* message;
} debug_t;

/*----------------------------- Private Functions ----------------------------*/

/**
 * @brief Internal function used to allocate memory for the error string.
 * @param length length of message passed in printf-style message.
 */
char* debug_pre_message(size_t length);

/**
 * @brief Internal function used add debug message to the queue.
 * @param debug debug struct that is passed to the queue.
 */
void debug_send_message(debug_t debug);

/*------------------------------ Public Functions ----------------------------*/

/**
 * @brief Add debug message to output queue.
 * @param debug_type debug message type - see Debug Types.
 * @param message printf-style arguments.
 */
#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL >= DEBUG_ERRORS
    #define DEBUG_MESSAGE(debug_type, raw_message) do { \
            debug_t debug; \
            debug.type = debug_type; \
            debug.message = pvPortMalloc(snprintf(NULL, 0, ESC(raw_message)));\
            sprintf(debug.message, ESC(raw_message)); \
            debug_send_message(debug); \
        } while(0)
#else
    #define DEBUG_MESSAGE(debug_type, message)
#endif /* DEBUG_LEVEL >= DEBUG_ERRORS */
#else
    /* The existence of DEBUG_LEVEL is only checked here. */
    #error "No Debug Level Defined!!!"
#endif /* DEBUG_LEVEL */

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
                            void (*send_func)(char), void (*reset_func)(void));

/**
 * @brief Suspend all tasks and halt everything for debugging.
 */
void debugFreeze(void);

/**
 * @brief Suspend the calling task for debugging.
 */
void debugFreezeTask(void);

/**
 * @brief Reset the ARM CPU.
 */
void debugReset(void);

#endif /* __FREERTOS_DEBUG__ */
