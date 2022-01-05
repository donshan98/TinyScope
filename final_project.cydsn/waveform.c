/* ======================================== *\
 *
 * Donald Shannon
 * dishanno@ucsc.edu
 * 1527177
 *
 * CSE 121
 * Lab Project - Waveform Functions
 *
 * manages the waveform data from the adc
 * 
\* ======================================== */

// STANDARD INCLUDES
#include <stdio.h>

// USER INCLUDES
#include "project.h"
#include "macros.h"

// GLOBAL VARIABLES
uint32_t channel_array = 2;
uint32_t array_ready = false;

// data for each channel with 2 arrays
uint16_t chA_data1[TRACK_TIME];
uint16_t chA_data2[TRACK_TIME];
uint16_t chB_data1[TRACK_TIME];
uint16_t chB_data2[TRACK_TIME];

// EXTERNAL FUNCTIONS
void throw_error(char* msg);
void uart_printf(char *print_string);


// FUNCTIONS

// initializes the dmas for the two adc channels
void init_dmas()
{
    // initializes the DMA
    Cy_DMA_Descriptor_Init(&DMA_A_Descriptor_1, &DMA_A_Descriptor_1_config);
    Cy_DMA_Descriptor_Init(&DMA_A_Descriptor_2, &DMA_A_Descriptor_2_config);
    
    Cy_DMA_Descriptor_Init(&DMA_B_Descriptor_1, &DMA_B_Descriptor_1_config);
    Cy_DMA_Descriptor_Init(&DMA_B_Descriptor_2, &DMA_B_Descriptor_2_config);
    
    
    // provides source and destination addresses
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_A_Descriptor_1, (uint32_t *) &(SAR->CHAN_RESULT[2]));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_A_Descriptor_1, chA_data1);
    
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_A_Descriptor_2, (uint32_t *) &(SAR->CHAN_RESULT[2]));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_A_Descriptor_2, chA_data2);
    
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_B_Descriptor_1, (uint32_t *) &(SAR->CHAN_RESULT[3]));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_B_Descriptor_1, chB_data1);
    
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_B_Descriptor_2, (uint32_t *) &(SAR->CHAN_RESULT[3]));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_B_Descriptor_2, chB_data2);
    
    
    // sets the loop counters to the size of the transfer
    Cy_DMA_Descriptor_SetXloopDataCount(&DMA_A_Descriptor_1, (uint32_t) TRACK_TIME);
    Cy_DMA_Descriptor_SetXloopDataCount(&DMA_A_Descriptor_2, (uint32_t) TRACK_TIME);
    
    Cy_DMA_Descriptor_SetXloopDataCount(&DMA_B_Descriptor_1, (uint32_t) TRACK_TIME);
    Cy_DMA_Descriptor_SetXloopDataCount(&DMA_B_Descriptor_2, (uint32_t) TRACK_TIME);
    
    // creates channel config
    cy_stc_dma_channel_config_t channelConfigA;
    channelConfigA.descriptor = &DMA_A_Descriptor_1;
    channelConfigA.preemptable = DMA_A_PREEMPTABLE;
    channelConfigA.priority = DMA_A_PRIORITY;
    channelConfigA.enable = false;
    channelConfigA.bufferable = DMA_A_BUFFERABLE;
    
    cy_stc_dma_channel_config_t channelConfigB;
    channelConfigB.descriptor = &DMA_B_Descriptor_1;
    channelConfigB.preemptable = DMA_B_PREEMPTABLE;
    channelConfigB.priority = DMA_B_PRIORITY;
    channelConfigB.enable = false;
    channelConfigB.bufferable = DMA_B_BUFFERABLE;
    
    
    // initializes DMA channel
    Cy_DMA_Channel_Init(DMA_A_HW, DMA_A_DW_CHANNEL, &channelConfigA);
    
    Cy_DMA_Channel_Init(DMA_B_HW, DMA_B_DW_CHANNEL, &channelConfigB);
    
    
    // enables DMA interrupt source
    Cy_DMA_Channel_SetInterruptMask(DMA_A_HW, DMA_A_DW_CHANNEL, CY_DMA_INTR_MASK);
    
    Cy_DMA_Channel_SetInterruptMask(DMA_B_HW, DMA_B_DW_CHANNEL, CY_DMA_INTR_MASK);
    
    
    // configures trigger mux
    //Cy_TrigMux_Connect(TRIG14_IN_PASS_TR_CTDAC_EMPTY, TRIG14_OUT_TR_GROUP0_INPUT50, CY_TR_MUX_TR_INV_DISABLE, TRIGGER_TYPE_PASS_TR_CTDAC_EMPTY);
    //Cy_TrigMux_Connect(TRIG1_IN_TR_GROUP14_OUTPUT7, TRIG1_OUT_CPUSS_DW1_TR_IN0, CY_TR_MUX_TR_INV_DISABLE, TRIGGER_TYPE_TR_GROUP_OUTPUT__LEVEL);
    
    // finally enables DMA
    Cy_DMA_Enable(DMA_A_HW);
    Cy_DMA_Enable(DMA_B_HW);
    
    // enables channel
    Cy_DMA_Channel_Enable(DMA_A_HW, DMA_A_DW_CHANNEL);
    Cy_DMA_Channel_Enable(DMA_B_HW, DMA_B_DW_CHANNEL);
}

// the isr for finishing both DMA descriptor
// tells the main function which array is ready
void dma_isr(void)
{
    if (channel_array == 1)
    {
        channel_array = 2;
        array_ready = true;
    }
    else if (channel_array == 2)
    {
        channel_array = 1;
        array_ready = true;
    }
    else
    {
        throw_error("ERROR: invalid channel array entry\n");
    }
    
    // clears interrupt
    NVIC_ClearPendingIRQ(dma_int_cfg.intrSrc);
    Cy_DMA_Channel_ClearInterrupt(DMA_A_HW, DMA_A_DW_CHANNEL);
}

// traverses the wave for two qualifying edges
// returns the two indeces for the two edges
// TODO: this can all be integrated into value pulling in main by tracking the last edge for each wave when it is pulled from the dma array
void get_endpoints(uint32_t* start, uint32_t* end, uint32_t trigger_slope, uint32_t trigger_level, uint16_t wave[])
{
    // checks for errors
    if ((start == NULL) | (end == NULL))
    {
        throw_error("ERROR: NULL input for get_endpoints\n");
    }
    
    // the first two consecutive qualifying edges in the wave
    uint32_t start_point = 0;
    uint32_t end_point = 0;
    
    // these keep track of what edge the loop is on and when to stop
    uint32_t firstedge = true;
    uint32_t done = false;
    
    // loops through the whole wave with extra space for the 5-point sampling
    for (uint32_t i = 3; i < (WAVESIZE-2); i++)
    {
        // pulls the weighted average of the current and last voltage value
        // only does the correct source
        uint32_t curr_val = 0;
        uint32_t last_val = 0;
        
        // does a weighted average of 5 values to reduce noise
        curr_val = (wave[i-2] + 2*wave[i-1] + 3*wave[i] + 2*wave[i+1] + wave[i+2]) / 9;
        last_val = (wave[i-3] + 2*wave[i-2] + 3*wave[i-1] + 2*wave[i] + wave[i+1]) / 9;
        
        // checks if there is a qualifying edge for positive edge
        if ((trigger_slope == POS_EDGE) & (last_val < trigger_level) & (curr_val >= trigger_level) & (!done))
        {
            if (firstedge)
            {
                // found the first edge
                start_point = i;
                firstedge = false;
            }
            else
            {
                // found end
                end_point = i;
                done = true;
            }
        }
        // checks for qualifying negative edge
        if ((trigger_slope == NEG_EDGE) & (last_val > trigger_level) & (curr_val <= trigger_level) & (!done))
        {
            if (firstedge)
            {
                // found the first edge
                start_point = i;
                firstedge = false;
            }
            else
            {
                // found end
                end_point = i;
                done = true;
            }
        }
    }
    *start = start_point;
    *end = end_point;
}


// returns a 2560 (10*samplesize) array of consecutive dma values
// pulls 2560 consecutive voltage data points from the dma
//(big enough to include a 100hz period at 250000 sample rate)
void get_chunk (uint16_t chA_wave[], uint16_t chB_wave[])
{
    // keeps track of how many arrays and position
    uint32_t arrays_added = 0;
    uint32_t currpos = 0;
    
    // pulls the correct number of arrays
    while (arrays_added < NUMSAMPLES)
    {
        // checks if the next array is ready from the DMA
        // otherwise keeps looping until one is ready
        if (array_ready)
        {
            if (channel_array == 1)//pulls from channel 1
            {
                // loops through all values of the array
                for (int i = 0; i < TRACK_TIME; i++)
                {
                    // checks for underflow
                    if (chA_data1[i] > MAX_ADC)
                    {
                        chA_wave[currpos] = 0;
                    }
                    else
                    {
                        chA_wave[currpos] = (chA_data1[i]) * MAX_mV / MAX_ADC;
                    }
                    
                    if (chB_data1[i] > MAX_ADC)
                    {
                        chB_wave[currpos] = 0;
                    }
                    else
                    {
                        chB_wave[currpos] = (chB_data1[i]) * MAX_mV / MAX_ADC;
                    }
                    currpos++;
                }
                arrays_added++;
            }
            else//pulls from channel 2
            {
                // loops through all values of the array
                for (int i = 0; i < TRACK_TIME; i++)
                {
                    // checks for underflow
                    if (chA_data2[i] > 0x7FF)
                    {
                        chA_wave[currpos] = 0;
                    }
                    else
                    {
                        chA_wave[currpos] = (chA_data2[i]) * MAX_mV / MAX_ADC;
                    }
                    
                    if (chB_data2[i] > 0x7FF)
                    {
                        chB_wave[currpos] = 0;
                    }
                    else
                    {
                        chB_wave[currpos] = (chB_data2[i]) * MAX_mV / MAX_ADC;
                    }
                    currpos++;
                }
                arrays_added++;
            }
            array_ready = false;
        }
    }
}

// returns the freqneucy using the start and endpoints
// assumes that start and end points cannot be 0
uint32_t get_freq(uint32_t start_point, uint32_t end_point)
{
    uint32_t freq = 0;
    if ((end_point != 0) & (start_point != 0))
    {
        freq = SAMPLES_PER_SEC / (end_point - start_point);
    }
    
    return freq;
}


/* [] END OF FILE */
