#include "pico/stdlib.h"
#include "pico-ssd1306/ssd1306.h"
#include "pico-ssd1306/textRenderer/TextRenderer.h"
#include "hardware/i2c.h"
#include <string.h>
#include <stdio.h>


// Use the namespace for convenience
using namespace pico_ssd1306;

SSD1306 init_screen(){
    // Init i2c0 controller
    i2c_init(i2c0, 1000000);
    // Set up pins 12 and 13
    gpio_set_function(0, GPIO_FUNC_I2C);
    gpio_set_function(1, GPIO_FUNC_I2C);
    // gpio_pull_up(12);
    // gpio_pull_up(13);

    // If you don't do anything before initializing a display pi pico is too fast and starts sending
    // commands before the screen controller had time to set itself up, so we add an artificial delay for
    // ssd1306 to set itself up
    sleep_ms(250);

    // Create a new display object at address 0x3D and size of 128x64
    SSD1306 display = SSD1306(i2c0, 0x3C, Size::W128xH64);

    // Here we rotate the display by 180 degrees, so that it's not upside down from my perspective
    // If your screen is upside down try setting it to 1 or 0
    display.setOrientation(0);
    return display;
}
void write_menu_on_screen(const char TITOLO[], const char sottotitolo1[], const char sottotitolo2[], const char sottotitolo3[], SSD1306 &display){
    int len = (128 - strlen(TITOLO)*12)/2;
    drawText(&display, font_12x16, TITOLO, len ,2);
    drawText(&display, font_5x8, sottotitolo1, 25 ,25);
    drawText(&display, font_8x8, "=>", 0 ,38);
    drawText(&display, font_8x8, sottotitolo2, 25 ,38);
    drawText(&display, font_5x8, sottotitolo3, 25 ,50);
    // Send buffer to the display
    display.sendBuffer();
}
int main(){
    SSD1306 display = init_screen();


    // Draw text on display
    // After passing a pointer to display, we need to tell the function what font and text to use
    // Available fonts are listed in textRenderer's readme
    // Last we tell this function where to anchor the text
    // Anchor means top left of what we draw
    const char TITOLO[] = "test123";
    const char sottotitolo1[] ="sottotitolo1";
    const char sottotitolo2[] ="0OOOOOOOOOOOOOOOO";
    const char sottotitolo3[] ="sottotitolo2";

    write_menu_on_screen(TITOLO,sottotitolo1,sottotitolo2,sottotitolo3, display);

    while (1)
    {
        printf("%d",strlen);
    }
    
}