#include "pico.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/time.h"

#include "buzzer.h"
#include "pitches.h"

int main()
{
  buzzer_init();

  click_button();
  sleep_ms(1000);
 end_menu();
 sleep_ms(1000);
 exit_nosave();
 sleep_ms(1000);
 exit_save();
 sleep_ms(1000);
 end_acquisition();
}
