/* ======================================== *\
 *
 * Donald Shannon
 * dishanno@ucsc.edu
 * 1527177
 *
 * CSE 121
 * Lab Project - Error Handler
 *
 * deals with errors
 * 
\* ======================================== */

// STANDARD INCLUDES

// USER INCLUDES
#include "project.h"

// DEFINITIONS

// GLOBAL VARIABLES

// EXTERNAL FUNCTIONS
void uart_printf(char *print_string);





// FUNCTIONS

// prints error and holds
void throw_error(char* msg)
{
    uart_printf(msg);
    while(true)
    {
        Cy_SysLib_Delay(1000);
    }
}

/* [] END OF FILE */
