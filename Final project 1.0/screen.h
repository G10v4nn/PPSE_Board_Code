#ifndef SCREEN_INCLUDED
#define SCREEN_INCLUDED

#include "ssd1306_font.h"
#include "Fonts/12x16_font.h"
#include "Fonts/16x32_font.h"
#include "Fonts/5x8_font.h"
#include "Fonts/8x8_font.h"

void init_screen(void);
void clear_screen(void);
void WriteString_Fonts_(int16_t x, int16_t y, char *str, const unsigned char *font);
void write_menu_on_screen(char TITOLO[],char sottotitolo1[],char sottotitolo2[],char sottotitolo3[]);

#endif