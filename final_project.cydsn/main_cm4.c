/* ======================================== *\
 *
 * Donald Shannon
 * dishanno@ucsc.edu
 * 1527177
 *
 * CSE 121
 * Lab Project - Main Function
 *
 * uses functions to create tinyscope
 * 
\* ======================================== */

// STANDARD INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// USER INCLUDES
#include "project.h"
#include "GUI.h"
#include "macros.h"
#include "mainincludes.h"


// GLOBAL VARIABLES
// mili-volts per division
uint32_t yscale = 1000;

// micro-seconds per division
// 1000 is one mili-second
uint32_t xscale = 1000;

// whether the program is sampling data at all
uint32_t keep_sampling = false;

// the mili-volt level that triggers an edge
uint32_t trigger_level = 1000;

// the type of slope that triggers an edge
uint32_t trigger_slope = POS_EDGE;

// the channel that triggers the edge
uint32_t trigger_source = CHAN_A;

// the mode in which data is displayed
uint32_t mode = FREE_RUN;

// the pixels per sample of the ADC
double pixels_per_sample = ((double) PIXELSPERX) * ((double) uS_PER_SAMPLE) / 1000.0;

// the number of samples to take from the wave at the current scale values
uint32_t numsamples = 0;


// MAIN LOOP
int main(void)
{
    
    // INTERRUPTS
    // creates DMA interrupt
    Cy_SysInt_Init(&dma_int_cfg, dma_isr);
    NVIC_EnableIRQ(dma_int_cfg.intrSrc);
    // enables interrupts
    __enable_irq();
    
    
    // VARIABLES
    // data for reading (in mV)
    // need at least 2500 for a 100hz period to fit
    uint16_t chA_wave[WAVESIZE];
    uint16_t chB_wave[WAVESIZE];
    
    // the y position for the two waveforms
    // centered on middle min=120
    uint32_t chA_ypos = 120;
    uint32_t chB_ypos = 120;
    
    // the frequency of each channel in hz
    uint32_t chA_freq[FREQS];
    uint32_t chB_freq[FREQS];
    uint32_t freq_it = 0;
    uint32_t freqAvgA = 0;
    uint32_t freqAvgB = 0;
    
    // the pixel values for the wave
    uint16_t wave_a_x[MAXPOINTS];
    uint16_t wave_a_y[MAXPOINTS];
    uint16_t wave_b_x[MAXPOINTS];
    uint16_t wave_b_y[MAXPOINTS];
    
    
    // the start and end points of one period of the wave
    uint32_t start_pointA = 0;
    uint32_t end_pointA = 0;
    uint32_t start_pointB = 0;
    uint32_t end_pointB = 0;
    
    // the string from the user terminal
    char *command;
    
    // a string for printing with sprintf
    char msg[64];
    
    
    // INITIALIZATION
    init_all();
    uart_printf("\n-- Tiny Scope --\n");
    
    
    // MAIN LOOP
    for(;;)
    {
        
        // poll commands line
        if (uart_scan() == TERM_COMPLETE)
        {
            // pulls the command
            command = uart_getstring();
            
            // sets relavent parameters
            parse_command(command);
            
            // frees command string
            free(command);
        }
        
        
        // poll the knobs
        uint32_t potA = 0;
        uint32_t potB = 0;
        get_pot(&potA, &potB);
        chA_ypos = potA * MAXY / MAX_ADC;
        chB_ypos = potB * MAXY / MAX_ADC;
        
        
        // only pulls new wave data if the scope is running
        if (keep_sampling)
        {
            // gets the voltage wave for each channel
            // pulls WAVESIZE consecutive voltage data points from the dma
            //(big enough to include a 100hz period at 250000 sample rate)
            get_chunk(chA_wave, chB_wave);
            
            // gets the qualifying edges for each wave
            // 0 if no edges found
            get_endpoints(&start_pointA, &end_pointA, trigger_slope, trigger_level, chA_wave);
            get_endpoints(&start_pointB, &end_pointB, trigger_slope, trigger_level, chB_wave);
            
            // adds the current frequency to the tally
            // current frequency will be the average of these
            // calculated in hz
            chA_freq[freq_it] = get_freq(start_pointA, end_pointA);
            chB_freq[freq_it] = get_freq(start_pointB, end_pointB);
            // iterates to the next frequency
            if (freq_it >= FREQS-1){freq_it = 0;}
            else{freq_it++;}
            
            freqAvgA = 0;
            freqAvgB = 0;
            // averages the new frequency values
            for (int i = 0; i < FREQS; i++)
            {
                freqAvgA += chA_freq[i];
                freqAvgB += chB_freq[i];
            }
            freqAvgA /= FREQS;
            freqAvgB /= FREQS;
        }
        
        
        // creates the pixel data for the wave
        // starts the data at the proper trigger point of zero if free running
        if (pixels_per_sample < 1.0)
        {
            for (uint32_t i = 0; i < numsamples; i++)
            {
                wave_a_x[i] = XMARGIN + i;
                wave_b_x[i] = XMARGIN + i;
                // starts from the trigger point set by the user
                if (mode == FREE_RUN)// free run
                {
                    wave_a_y[i] = chA_wave[(int) (i / pixels_per_sample)] * PIXELSPERY / yscale;
                    wave_b_y[i] = chB_wave[(int) (i / pixels_per_sample)] * PIXELSPERY / yscale;
                }
                else if (trigger_source == CHAN_A)// trigger channel a
                {
                    wave_a_y[i] = chA_wave[start_pointA + ((int) (i / pixels_per_sample))] * PIXELSPERY / yscale;
                    wave_b_y[i] = chB_wave[start_pointA + ((int) (i / pixels_per_sample))] * PIXELSPERY / yscale;
                }
                else// trigger channel b
                {
                    wave_a_y[i] = chA_wave[start_pointB + ((int) (i / pixels_per_sample))] * PIXELSPERY / yscale;
                    wave_b_y[i] = chB_wave[start_pointB + ((int) (i / pixels_per_sample))] * PIXELSPERY / yscale;
                }
            }
        }
        else
        {
            for (uint32_t i = 0; i < numsamples; i++)
            {
                wave_a_x[i] = XMARGIN + (i * pixels_per_sample);
                wave_b_x[i] = XMARGIN + (i * pixels_per_sample);
                // starts from the trigger point set by the user
                if (mode == FREE_RUN)// free run
                {
                    wave_a_y[i] = chA_wave[i] * PIXELSPERY / yscale;
                    wave_b_y[i] = chB_wave[i] * PIXELSPERY / yscale;
                }
                else if (trigger_source == CHAN_A)// trigger channel a
                {
                    wave_a_y[i] = chA_wave[start_pointA + i] * PIXELSPERY / yscale;
                    wave_b_y[i] = chB_wave[start_pointA + i] * PIXELSPERY / yscale;
                }
                else// trigger channel b
                {
                    wave_a_y[i] = chA_wave[start_pointB + i] * PIXELSPERY / yscale;
                    wave_b_y[i] = chB_wave[start_pointB + i] * PIXELSPERY / yscale;
                }
            }
        }
        
        // redraw background
        drawBackground(XSIZE, YSIZE, XMARGIN, YMARGIN);
        
        // redraw grid
        drawGrid(XSIZE, YSIZE, XDIVISIONS, YDIVISIONS, XMARGIN, YMARGIN);
        
        // draws the waves
        drawWave(wave_a_x, wave_a_y, numsamples, chA_ypos, GUI_ORANGE);
        drawWave(wave_b_x, wave_b_y, numsamples, chB_ypos, GUI_BLUE);
        
        // prints scale
        printScaleSettings(xscale, yscale, 200, 15);
        
        // prints frequency
        printFrequency(freqAvgA, freqAvgB, 10, 15);
        
        // ideally refreshes display at this period
        // 100 = 10 hz
        // with calculation time its actually way slower
        Cy_SysLib_Delay(100);
        
    }
}

// FUNCTIONS

// does all the initialization stuff
void init_all()
{
    init_terminal();
    
    GUI_Init();
    
    ShowStartupScreen();
    
    init_dmas();
    
    init_adc();
    
    Cy_SysLib_Delay(1000);
}

/* [] END OF FILE */
