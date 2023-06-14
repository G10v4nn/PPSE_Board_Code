#ifndef BUZZER_INCLUDED
#define BUZZER_INCLUDED

void init_buzzer(void);
void click_button(void);
void end_menu(void);
void exit_nosave(void);
void exit_save(void);
void end_acquisition(void);
void play(uint16_t frequency);
void stop(void);

#endif