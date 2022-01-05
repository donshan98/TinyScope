/* ======================================== *\
 *
 * Donald Shannon
 * dishanno@ucsc.edu
 * 1527177
 *
 * CSE 121
 * Lab Project - Main Includes
 *
 * functions needed for main
 * 
\* ======================================== */

#include <stdint.h>

#include "GUI.h"

#ifndef MAIN_INCLUDES
#define MAIN_INCLUDES
    
    // from main
    void init_all();
    
    // from terminal
    void init_terminal(void);
    void uart_printf(char *print_string);
    uint32_t uart_scan(void);
    char* uart_getstring(void);
    void parse_command(char* command);
    
    // from errors
    void throw_error(char* msg);
    
    // from knobs
    void init_adc(void);
    void get_pot (uint32_t *potA, uint32_t *potB);
    
    // from waveform
    void init_dmas();
    void dma_isr(void);
    void get_endpoints(uint32_t* start, uint32_t* end, uint32_t trigger_slope, uint32_t trigger_level, uint16_t wave[]);
    void get_chunk (uint16_t chA_wave[], uint16_t chB_wave[]);
    uint32_t get_freq(uint32_t start_point, uint32_t end_point);
    
    // from graphics
    void ShowStartupScreen(void);
    void printFrequency(int xfreq, int yfreq, int xposition, int yposition);
    void drawWave(uint16_t waveX[], uint16_t waveY[], uint32_t numsamples, uint32_t ypos, uint32_t color);
    void printScaleSettings(int xscale, int yscale, int xposition, int yposition);
    void drawGrid(int w, int h, int xdiv, int ydiv, int xmargin, int ymargin);
    void drawBackground(int w, int h, int xmargin, int ymargin);
    
    // GLOBAL VARIABLES
    extern uint32_t channel_array;
    extern uint32_t array_ready;
    
#endif


/* [] END OF FILE */
