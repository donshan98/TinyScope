/* ======================================== *\
 *
 * Donald Shannon
 * dishanno@ucsc.edu
 * 1527177
 *
 * CSE 121
 * Lab Project - Command Parser
 *
 * initializes, parses, and prints to terminal
 * 
\* ======================================== */

// STANDARD INCLUDES
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// USER INCLUDES
#include "project.h"
#include "macros.h"

// GLOBAL VARIABLES
// input string stuff
char input_string[128] = "\0";
uint32_t status = TERM_EMPTY;
uint32_t input_index = 0;

// mili-volts per division
extern uint32_t yscale;

// micro-seconds per division
// 1000 is one mili-second
extern uint32_t xscale;

// whether the program is sampling data at all
extern uint32_t keep_sampling;

// the mili-volt level that triggers an edge
extern uint32_t trigger_level;

// the type of slope that triggers an edge
extern uint32_t trigger_slope;

// the channel that triggers the edge
extern uint32_t trigger_source;

// the mode in which data is displayed
extern uint32_t mode;

// number of pixels per sample of the ADC
extern double pixels_per_sample;

// the number of samples to take at the current scale value
extern uint32_t numsamples;

// EXTERNAL FUNCTIONS
void throw_error(char* msg);




// FUNCTIONS

// initializes the UART terminal
void init_terminal(void)
{
    // starts the uart
    Cy_SCB_UART_Init(UART_printf_HW, &UART_printf_config, &UART_printf_context);
    Cy_SCB_UART_Enable(UART_printf_HW);
}

// prints to the UART terminal
void uart_printf(char *print_string)
{
    Cy_SCB_UART_PutString(UART_printf_HW, print_string);
}

// gets all the available chars in the uart buffer
// returns status of 1 for finished string 0 otherwise
// checks for errors in uart
uint32_t uart_scan(void)
{
    // keeps reading chars from uart until the buffer is empty
    uint32_t RXstatus = Cy_SCB_UART_GetRxFifoStatus(UART_printf_HW);
    while(RXstatus & CY_SCB_UART_RX_NOT_EMPTY)
    {
        // handles the errors
        if (RXstatus & CY_SCB_UART_RX_ERR_PARITY)
        {
            uart_printf("Warning: terminal input parity error\n");
            Cy_SCB_UART_ClearRxFifoStatus(UART_printf_HW, CY_SCB_UART_RX_ERR_PARITY);
        }
        if (RXstatus & CY_SCB_UART_RX_ERR_FRAME)
        {
            uart_printf("Warning: terminal input framing error\n");
            Cy_SCB_UART_ClearRxFifoStatus(UART_printf_HW, CY_SCB_UART_RX_ERR_FRAME);
        }
        
        // wont load string if current string is complete
        if (status == TERM_COMPLETE)
        {
            uart_printf("WARNING: trying to override existing input string\n");
            break;
        }
        
        // loads string
        input_string[input_index] = Cy_SCB_UART_Get(UART_printf_HW);
        status = TERM_PENDING;
        
        // checks if the string is done
        // putty terminates strings with '\r'
        if (input_string[input_index] == '\r')
        {
            input_string[input_index] = '\0';
            status = TERM_COMPLETE;
        }
        
        // increments index
        input_index++;
        
        // clears the not empty flag
        Cy_SCB_UART_ClearRxFifoStatus(UART_printf_HW, CY_SCB_UART_RX_NOT_EMPTY);
        // checks if it is still not empty
        RXstatus = Cy_SCB_UART_GetRxFifoStatus(UART_printf_HW);
    }
    
    return status;
}

// returns the string from the terminal input if it exists
// REMEMBER TO DEALLOC AFTER USE
char* uart_getstring(void)
{
    // throws and error if the string is not finished
    if (status != TERM_COMPLETE)
    {
        throw_error("ERROR: tried to pull unfinished string in uart_getstring\n");
    }
    // copies the finished string
    char *new_string = calloc(128, sizeof(char));
    strcpy(new_string, input_string);
    
    // resets the input
    strcpy(input_string, "\0");
    status = TERM_EMPTY;
    input_index = 0;
    
    // returns the finished string
    return new_string;
}

// parses the command string and sets relevant parameters
void parse_command(char* command)
{
    // checks for broken command pointer
    if (command == NULL)
    {
        throw_error("ERROR: invalid command pointer in parse_command\n");
    }
    
    // the message to print when using sprintf
    char msg[64];
    
    // starts parsing the command
    if (!strcasecmp(command, "ping"))// check for response
    {
        uart_printf("pong!\n");
    }
    else if (!strcasecmp(command, "start"))// scope is started
    {
        keep_sampling = true;
        // changes number of samples
        if (pixels_per_sample < 1.0)
        {
            numsamples = MAXPOINTS;
        }
        else
        {
            numsamples = MAXPOINTS/pixels_per_sample;
        }
        uart_printf("Scope started.\n");
    }
    else if (!strcasecmp(command, "stop"))// scope is stopped
    {
        keep_sampling = false;
        uart_printf("Scope stopped.\n");
    }
    else if (!strcasecmp(command, "set mode free"))// mode switch to free if the scope is stopped
    {
        if (keep_sampling)
        {
            uart_printf("Scope must be stopped to change mode.\n");
        }
        else
        {
            mode = FREE_RUN;
            uart_printf("Mode set to: Free-Running\n");
        }
    }
    else if (!strcasecmp(command, "set mode trigger"))// mode switch to trigger if the scope is stopped
    {
        if (keep_sampling)
        {
            uart_printf("Scope must be stopped to change mode.\n");
        }
        else
        {
            mode = TRIGGER;
            uart_printf("Mode set to: Trigger\n");
        }
    }
    else if (!strncasecmp(command, "set trigger_level ", 18))// sets the trigger level in mV if the scope is stopped
    {
        if (keep_sampling)
        {
            uart_printf("Scope must be stopped to change trigger level.\n");
        }
        else
        {
            // position 19 should be the start of the number
            uint32_t newLevel = atoi(&command[18]);
            
            // casts out the lower 2 digits
            newLevel /= 100;
            newLevel *= 100;
            
            if (newLevel > 3000)
            {
                uart_printf("Invalid trigger level.");
            }
            else
            {
                trigger_level = newLevel;
            }
            
            sprintf(msg, "Set trigger level to: %d\n", trigger_level);
            uart_printf(msg);
        }
    }
    else if (!strcasecmp(command, "set trigger_slope positive"))// sets the trigger slope to positive
    {
        if (keep_sampling)
        {
            uart_printf("Scope must be stopped to change trigger slope.\n");
        }
        else
        {
            trigger_slope = POS_EDGE;
            uart_printf("Trigger slope set to: positive edge\n");
        }
    }
    else if (!strcasecmp(command, "set trigger_slope negative"))// sets the trigger slope to negative
    {
        if (keep_sampling)
        {
            uart_printf("Scope must be stopped to change trigger slope.\n");
        }
        else
        {
            trigger_slope = NEG_EDGE;
            uart_printf("Trigger slope set to: negative edge\n");
        }
    }
    else if (!strcasecmp(command, "set trigger_channel 1") | !strcasecmp(command, "set trigger_channel a"))// sets the trigger channel to channel A
    {
        if (keep_sampling)
        {
            uart_printf("Scope must be stopped to change trigger channel.\n");
        }
        else
        {
            trigger_source = CHAN_A;
            uart_printf("Trigger channel set to: Channel A\n");
        }
    }
    else if (!strcasecmp(command, "set trigger_channel 2") | !strcasecmp(command, "set trigger_channel b"))// sets the trigger channel to channel B
    {
        if (keep_sampling)
        {
            uart_printf("Scope must be stopped to change trigger channel.\n");
        }
        else
        {
            trigger_source = CHAN_B;
            uart_printf("Trigger channel set to: Channel B\n");
        }
    }
    else if (!strncasecmp(command, "set xscale ", 11))// changes the xscale
    {
        // position 12 should be the start of the number
        uint32_t newxScale = atoi(&command[11]);
        
        // casts out the lower 2 digits
        newxScale /= 100;
        
        if (newxScale == 1)
        {
            xscale = 100;
        }
        else if (newxScale == 2)
        {
            xscale = 200;
        }
        else if (newxScale == 5)
        {
            xscale = 500;
        }
        else if (newxScale == 10)
        {
            xscale = 1000;
        }
        else if (newxScale == 20)
        {
            xscale = 2000;
        }
        else if (newxScale == 50)
        {
            xscale = 5000;
        }
        else if (newxScale == 100)
        {
            xscale = 10000;
        }
        else
        {
            uart_printf("Invalid xscale value.\n");
        }
        
        // sets the new pixels per sample for the new xscale
        pixels_per_sample = ((double) PIXELSPERX) * ((double) uS_PER_SAMPLE) / ((double) xscale);
        // changes number of samples
        if (pixels_per_sample < 1.0)
        {
            numsamples = MAXPOINTS;
        }
        else
        {
            numsamples = MAXPOINTS/pixels_per_sample;
        }
        
        sprintf(msg, "Set xscale to: %d\n", xscale);
        uart_printf(msg);
    }
    else if (!strncasecmp(command, "set yscale ", 11))// changes the yscale
    {
        // position 12 should be the start of the number
        uint32_t newyScale = atoi(&command[11]);
        
        // casts out the lower 2 digits
        newyScale /= 100;
        
        if (newyScale == 5)
        {
            yscale = 500;
        }
        else if (newyScale == 10)
        {
            yscale = 1000;
        }
        else if (newyScale == 15)
        {
            yscale = 1500;
        }
        else if (newyScale == 20)
        {
            yscale = 2000;
        }
        else
        {
            uart_printf("Invalid yscale value.\n");
        }
        
        sprintf(msg, "Set yscale to: %d\n", yscale);
        uart_printf(msg);
    }
    else
    {
        uart_printf("Invalid command.\n");
    }
}


/* [] END OF FILE */
