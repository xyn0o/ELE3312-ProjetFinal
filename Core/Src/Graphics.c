/**
 * @file Grphics.c
 * @brief This module contains some helper functions to draw different geometric 
 *        shapes as well as other more complex arangements (like the Ghost and Pac-Man).
 */
#include "Graphics.h"

/**
* @brief Draws a simple representation of a grave stone on the screen.
* @param lcd [in] pointer to the ili9341 handle structure used to access the tft screen
* @param x [in] integer representing the x coordinate of the lower-left corner of the image
* @param y [in] integer representing the y coordinate of the lower-left corner of the image
*/
void drawGrave(ili9341_t *lcd, int16_t x, int16_t y){
	x -= 12;
	y += 5;
	const ili9341_color_t grave_color = ILI9341_DARKGREY;
	// Body
	ili9341_fill_circle(lcd, grave_color, x + 61, y-70,51);
	ili9341_fill_rect(lcd, grave_color, x+10, y-80, 104, 70);
	ili9341_fill_rect(lcd, grave_color, x, y-30, 124, 35);
	char msg[] = "R.I.P.";
	ili9341_text_attr_t text_attr = {&ili9341_font_11x18, ILI9341_BLACK, grave_color, x + 30, y - 60};
	ili9341_draw_string(lcd, text_attr, msg);
}

// Draw pacman
// From Arduino library
// https://github.com/moononournation/Arduino_GFX/blob/master/src/Arduino_GFX.cpp
/**
* @brief Helper function to draw filled segments of a circle more details can be found [online](https://github.com/moononournation/Arduino_GFX/blob/master/src/Arduino_GFX.cpp).
*/
void writeFillArcHelper(ili9341_t *lcd, int16_t cx, int16_t cy, int16_t oradius, int16_t iradius, float start, float end, ili9341_color_t color)
{
	const float DEGTORAD = 0.017453292519943295769236907684886F;
  if ((start == 90.0) || (start == 180.0) || (start == 270.0) || (start == 360.0))
  {
    start -= 0.1;
  }

  if ((end == 90.0) || (end == 180.0) || (end == 270.0) || (end == 360.0))
  {
    end -= 0.1;
  }

  float s_cos = (cos(start * DEGTORAD));
  float e_cos = (cos(end * DEGTORAD));
  float sslope = s_cos / (sin(start * DEGTORAD));
  float eslope = e_cos / (sin(end * DEGTORAD));
  float swidth = 0.5 / s_cos;
  float ewidth = -0.5 / e_cos;
  --iradius;
  int32_t ir2 = iradius * iradius + iradius;
  int32_t or2 = oradius * oradius + oradius;

  uint16_t start180 = !(start < 180.0);
  uint16_t end180 = end < 180.0;
  uint16_t reversed = start + 180.0 < end || (end < start && start < end + 180.0);

  int32_t xs = -oradius;
  int32_t y = -oradius;
  int32_t ye = oradius;
  int32_t xe = oradius + 1;
  if (!reversed)
  {
    if ((end >= 270 || end < 90) && (start >= 270 || start < 90))
    {
      xs = 0;
    }
    else if (end < 270 && end >= 90 && start < 270 && start >= 90)
    {
      xe = 1;
    }
    if (end >= 180 && start >= 180)
    {
      ye = 0;
    }
    else if (end < 180 && start < 180)
    {
      y = 0;
    }
  }
  do
  {
    int32_t y2 = y * y;
    int32_t x = xs;
    if (x < 0)
    {
      while (x * x + y2 >= or2)
      {
        ++x;
      }
      if (xe != 1)
      {
        xe = 1 - x;
      }
    }
    float ysslope = (y + swidth) * sslope;
    float yeslope = (y + ewidth) * eslope;
    int32_t len = 0;
    do
    {
      uint16_t flg1 = start180 != (x <= ysslope);
      uint16_t flg2 = end180 != (x <= yeslope);
      int32_t distance = x * x + y2;
      if (distance >= ir2 && ((flg1 && flg2) || (reversed && (flg1 || flg2))) && x != xe && distance < or2)
      {
        ++len;
      }
      else
      {
        if (len)
        {
          //writeFastHLine(cx + x - len, cy + y, len, color);
          ili9341_draw_line(lcd, color, cx + x - len, cy + y, cx + x, cy + y);
          len = 0;
        }
        if (distance >= or2)
          break;
        if (x < 0 && distance < ir2)
        {
          x = -x;
        }
      }
    } while (++x <= xe);
  } while (++y <= ye);
}

/**
* @brief This function draws a filled segment of a circle on the screen. The original code can be found [online] (https://github.com/moononournation/Arduino_GFX/blob/master/src/Arduino_GFX.cpp)
* @param lcd [in]  pointer to the ili9341 handle structure used to access the tft screen
* @param x [in] integer representing the circles' center x position
* @param y [in] integer representing the circles' center y position
* @param r1 [in] integer representing the start of the radius
* @param r2 [in] integer representing the end of the radius
* @param start [in] float value of the start angle
* @param end [in] flaot value of the end angle
* @param color [in] ili9341_color_t color value of the filled segment of the circle
*/
void fillArc(ili9341_t *lcd, int16_t x, int16_t y, int16_t r1, int16_t r2, float start, float end, ili9341_color_t color)
{
	const float FLT_EPSILON = 0.00001;
  if (r1 < r2)
  {
    //_swap_int16_t(r1, r2);
	  int16_t t = r1;
	  r1 = r2;
	  r2 = t;
  }
  if (r1 < 1)
  {
    r1 = 1;
  }
  if (r2 < 1)
  {
    r2 = 1;
  }
  int equal = (int)( fabs(start - end) < FLT_EPSILON );
  start = fmodf(start, 360);
  end = fmodf(end, 360);
  if (start < 0)
    start += 360.0;
  if (end < 0)
    end += 360.0;
  if (!equal && (fabsf(start - end) <= 0.0001))
  {
    start = .0;
    end = 360.0;
  }

  //startWrite();
  writeFillArcHelper(lcd, x, y, r1, r2, start, end, color);
  //endWrite();
}

/**
* @brief Function that draws a representation of Pac-Man
* @param lcd [in]  pointer to the ili9341 handle structure used to access the tft screen
* @param x [in] unsigned integer of the x position of the lower-left corner of the image
* @param y [in] unsigned integer of the y position of the lower-left corner of the image
* @param color [in]  ili9341_color_t color of the figure
*/
void drawPacMan(ili9341_t *lcd, uint16_t x, uint16_t y, ili9341_color_t color){
	const uint16_t radius = 50;
	// Draw Body
	fillArc(lcd, x + radius, y - radius, 0, radius, -150, 150, color);// y = 120, x = 200
	// Draw Eye
	ili9341_fill_circle(lcd, ILI9341_WHITE, x + radius,y - radius - 25, 8);
	ili9341_fill_circle(lcd, ILI9341_BLACK, x + radius - 2, y - radius - 28, 2);// y = 92
}

/**
* @brief Function that draws a representation of the Ghost
* @param lcd [in]  pointer to the ili9341 handle structure used to access the tft screen
* @param x [in] unsigned integer of the x position of the lower-left corner of the image
* @param y [in] unsigned integer of the y position of the lower-left corner of the image
* @param color [in]  ili9341_color_t color of the figure
*/
void drawGhost(ili9341_t *lcd, uint16_t x, uint16_t y, ili9341_color_t color){
	y -= 20; // Shift ghost so that it (x, y) give the lower left corner of the ghost
	// Body
	ili9341_fill_circle(lcd,color,x+49,y-40,50);
	ili9341_fill_rect(lcd,color, x, y-40, 100, 50); //40, 80, 80, 40
	ili9341_fill_circle(lcd,color,x+20,y+5,20); //53,120,13
	ili9341_fill_circle(lcd,color,x+50,y+5,20);//79,120,13
	ili9341_fill_circle(lcd,color,x+79,y+5,20); //105,120,13
	// Eyes
	ili9341_fill_circle(lcd,ILI9341_WHITE,x+25,y-40,8);
	ili9341_fill_circle(lcd,ILI9341_WHITE,x+65,y-40,8);
	// Pupils
	ili9341_fill_circle(lcd,ILI9341_BLACK,x+27,y-38,2);//67,y-2,2
	ili9341_fill_circle(lcd,ILI9341_BLACK,x+63,y-42,2);//93,y+2,2
}

