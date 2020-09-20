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

/*----------------------------- Private Functions ----------------------------*/

/** 
 * @brief Internal function used to allocate memory for the error string.
 * @param length length of message passed in printf-style message.
*/
char* debug_pre_message(size_t length)
{

}

/** 
 * @brief Internal function used add debug message to the queue.
 * @param debug_type debug message type - see Debug Types.
 * @param length length of message passed in printf-style message.
*/
void debug_post_message(debug_t debug)
{

}

/*------------------------------ Public Functions ----------------------------*/

/** 
 * @brief Suspend all tasks and halt everything for debugging.
*/
void debugFreeze(void)
{

}

/** 
 * @brief Suspend the calling task for debugging.
*/
void debugFreezeTask(void)
{

}

/** 
 * @brief Reset the ARM CPU.
*/
void debugReset(void)
{
    
}
