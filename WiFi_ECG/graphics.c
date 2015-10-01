/**
 * Graphics related functions
 */
 

#include "setup.h"
#include "graphics.h"
#include "firmware.h"

/*-----------------------------------------------------------*/
 
 
 
/* LCD Module init */
 void initDisplay (void) 
{
	
	GLCD_init();
	GLCD_clear(White);
	//GLCD_setTextColor(Black);
	//GLCD_drawLine(120, 0, 320, Horizontal);
	
}

/*-----------------------------------------------------------*/


/*Draw a line between two given points using Bresenham's line algorithm*/
void Draw_line (int x0, int y0, int x1,  int y1)
{
    int dx = abs(x1-x0);
    int dy = abs(y1-y0);
    signed short sx,sy;
    signed int err,e2;

    if (x0 < x1) {sx = 1;} else {sx = -1;}
    if (y0 < y1) {sy = 1;} else {sy = -1;}
    err = dx-dy;

    while (!(x0==x1 && y0==y1))
    {
        GLCD_putPixel(y0, x0);
        e2 = 2*err;
        if (e2 > -dy) 
        {
            err = err - dy;
            x0 += sx;
        }
        if (e2 < dx) 
        {
            err = err + dx;
            y0 += sy;
        }
    }
}

