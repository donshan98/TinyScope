/* ======================================== *\
 *
 * Donald Shannon
 * dishanno@ucsc.edu
 * 1527177
 *
 * CSE 121
 * Lab Project - Graphics Functions
 *
 * prints graphics and stuff
 * much is borrowed from Anujan Varma's Final Project Demo
 * 
\* ======================================== */

// STANDARD INCLUDES
#include <stdio.h>

// USER INCLUDES
#include "project.h"
#include "GUI.h"
#include "macros.h"

// GLOBAL VARIABLES

// EXTERNAL FUNCTIONS
void throw_error(char* msg);



// FUNCTIONS

// shows a startup screen
// borrowed from Anujan Varma's Final Project Demo
void ShowStartupScreen(void)
{
    /* Set font size, foreground and background colors */
    GUI_SetFont(GUI_FONT_32B_1);
    GUI_SetBkColor(GUI_BLUE);
    GUI_SetColor(GUI_LIGHTMAGENTA);
    GUI_Clear();
  
    GUI_SetTextAlign(GUI_TA_HCENTER | GUI_TA_VCENTER);
    GUI_DispStringAt("TinyScope", 160, 120);
    
    GUI_SetFont(GUI_FONT_24B_1);
    GUI_SetColor(GUI_LIGHTGREEN);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("CSE 121/L Final Project", 160, 200);
}

// prints the frequency for channel a and b
// borrowed from Anujan Varma's Final Project Demo
void printFrequency(int xfreq, int yfreq, int xposition, int yposition)
{ 
    char str[20];

    GUI_SetBkColor(GUI_DARKCYAN); // Set background color
    GUI_SetFont(GUI_FONT_16B_1); // Set font
    GUI_SetColor(GUI_LIGHTGRAY); // Set foreground color

    sprintf(str, "Ch 1 Freq: %0d Hz", xfreq);
    GUI_DispStringAt(str, xposition, yposition);
    sprintf(str, "Ch 2 Freq: %0d Hz", yfreq);
    GUI_DispStringAt(str, xposition, yposition + 20);
}

// plots the waveform with y pos
void drawWave(uint16_t waveX[], uint16_t waveY[], uint32_t numsamples, uint32_t ypos, uint32_t color)
{
    GUI_SetPenSize(1);
    GUI_SetColor(color);
    GUI_SetLineStyle(GUI_LS_SOLID);
    
    for (uint32_t i = 1; i < numsamples; i++)
    {
        GUI_DrawLine(waveX[i-1], ypos - waveY[i-1], waveX[i], ypos - waveY[i]);
    }
}

// shows the scale settings on the screen
// borrowed from Anujan Varma's Final Project Demo
void printScaleSettings(int xscale, int yscale, int xposition, int yposition)
{
    char str[20];

    GUI_SetBkColor(GUI_DARKCYAN); // Set background color
    GUI_SetFont(GUI_FONT_16B_1); // Set font
    GUI_SetColor(GUI_LIGHTGRAY); // Set foreground color
    
    if (xscale >= 1000)
        sprintf(str, "Xscale: %0d ms/div", xscale/1000);
    else
        sprintf(str, "Xscale: %0d us/div", xscale);
    
    GUI_DispStringAt(str, xposition, yposition);
    int yscaleVolts = yscale/1000;
    int yscalemVolts = yscale % 1000;
    
    if (yscale >= 1000)
        sprintf(str, "Yscale: %0d V/div", yscaleVolts);
    else
        sprintf(str, "Yscale: %0d.%0d V/div", yscaleVolts, yscalemVolts/100);
    GUI_DispStringAt(str, xposition, yposition + 20);
}

// draws the x-y grid
// borrowed from Anujan Varma's Final Project Demo
void drawGrid(int w, int h, int xdiv, int ydiv, int xmargin, int ymargin)
{
    int xstep = (w-xmargin*2)/xdiv;
    int ystep = (h-ymargin*2)/ydiv;

    GUI_SetPenSize(1);
    GUI_SetColor(GUI_LIGHTGRAY);
    GUI_SetLineStyle(GUI_LS_DOT);
    
    for (int i=1; i<ydiv; i++){
        GUI_DrawLine(xmargin, ymargin+i*ystep, w-xmargin, ymargin+i*ystep);
    }
    for (int i=1; i<xdiv; i++){
        GUI_DrawLine(xmargin+i*xstep, ymargin, xmargin+i*xstep, h-ymargin);
    }
}

void drawBackground(int w, int h, int xmargin, int ymargin)
{
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetColor(GUI_DARKCYAN);
    GUI_FillRect(0, 0, w, h);
    GUI_SetPenSize(1);
    GUI_SetColor(GUI_LIGHTGRAY);
    GUI_DrawRect(xmargin, ymargin, w-xmargin, h-ymargin);
    GUI_SetLineStyle(GUI_LS_DOT);
}

/* [] END OF FILE */
