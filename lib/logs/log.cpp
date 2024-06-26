#include <TFT_eSPI.h>

#include <colors.h>
#include "log.h"

extern TFT_eSPI tft;

void header_msg(const char* msg)
{
    tft.fillRect(0, 0, 240, 25, YELLOWCLEAR);
    tft.setCursor(5,5,2);
    tft.print( msg);
}