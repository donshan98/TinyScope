/* ======================================== *\
 *
 * Donald Shannon
 * dishanno@ucsc.edu
 * 1527177
 *
 * CSE 121
 * Lab Project - macros
 *
 * macros and defines for my program
 * 
\* ======================================== */

#ifndef MACROS_HEADER
#define MACROS_HEADER
    
    #define TERM_EMPTY 0
    #define TERM_PENDING 1
    #define TERM_COMPLETE 2
    
    #define MAX_ADC 0x7FF
    
    #define MAX_mV 3300
    
    #define TRACK_TIME 256 // Max 256 for dma
    
    #define CHAN_A 0
    #define CHAN_B 1
    
    #define POS_EDGE 0
    #define NEG_EDGE 1
    
    #define FREE_RUN 0
    #define TRIGGER 1
    
    #define SAMPLES_PER_SEC 232000
    #define uS_PER_SAMPLE (1000000/SAMPLES_PER_SEC)
    
    #define NUMSAMPLES 98 // enough ADC sample arrays to accomodate both xscale 10000 and 1 period of 100hz signal
    #define WAVESIZE TRACK_TIME*NUMSAMPLES
    
    #define XSIZE 320 // Width of LCD screen in pixels
    #define YSIZE 240 // Height of LCF screen in pixels
    #define XDIVISIONS 10 // Number of x-axis divisions for grid
    #define YDIVISIONS 8 // Number of y-axis divisions for grid
    #define XMARGIN 5 // Margin around screen on x-axis
    #define YMARGIN 4 // Margin around screen on y-axis
    #define MAXPOINTS (XSIZE-(2*XMARGIN)) // Maximum number of points in wave for plotting
    #define MAXY (YSIZE-(2*YMARGIN))
    #define PIXELSPERX (MAXPOINTS/XDIVISIONS)
    #define PIXELSPERY (MAXY/YDIVISIONS)

    #define PI 3.14159265
    
    #define FREQS 5
    
    #define debug true
    
#endif

/* [] END OF FILE */
