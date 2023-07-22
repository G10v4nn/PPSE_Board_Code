#ifndef SCREEN_INCLUDED
#define SCREEN_INCLUDED

void init_screen(void);
void clear_screen(void);
void WriteString_Fonts_8x8(int16_t x, int16_t y, char *str);
void WriteString_Fonts_12x16(int16_t x, int16_t y, char *str);
void write_menu_on_screen(char TITOLO[],char sottotitolo1[],char sottotitolo2[],char sottotitolo3[]);

#endif