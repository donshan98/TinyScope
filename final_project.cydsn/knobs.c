/* ======================================== *\
 *
 * Donald Shannon
 * dishanno@ucsc.edu
 * 1527177
 *
 * CSE 121
 * Lab Project - Potentiometer Knobs
 *
 * controls the potentiometer knobs
 * 
\* ======================================== */

// STANDARD INCLUDES

// USER INCLUDES
#include "project.h"
#include "macros.h"

// GLOBAL VARIABLES

// EXTERNAL FUNCTIONS
void throw_error(char* msg);



// FUNCTIONS

// initializes the knobs
void init_adc(void)
{
    ADC_1_Start();
    ADC_1_StartConvert();
}

// returns the two potentiometer values (0 - 7FF)
void get_pot (uint32_t *potA, uint32_t *potB)
{
    // checks for errors
    if ((potA == NULL) | (potB == NULL))
    {
        throw_error("ERROR: NULL input for get_pot\n");
    }
    
    // pulls the raw value
    uint32_t rawVa = (uint32_t) ADC_1_GetResult32(0);
    uint32_t rawVb = (uint32_t) ADC_1_GetResult32(1);
    
    // checks for underflow
    if (rawVa > MAX_ADC)
    {
        *potA = 0;
    }
    else
    {
        *potA = rawVa;
    }
    
    if (rawVb > MAX_ADC)
    {
        *potB = 0;
    }
    else
    {
        *potB = rawVb;
    }
}

/* [] END OF FILE */
