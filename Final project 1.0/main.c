/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*****************************               DESIGN                      ****************
 * 
 *      l'applicazione dopo l'inizializzazione parte da un menù di selezione per scegliere cosa fare
 *      le opzioni sono:
 *          - far partire l'acquisizione dati con i valori di default
 *          - scegliere il settings dell'acquisizione dei dati
 *          - scaricare il file con i dati fino ad ora salvati
 *      
 *      In questo menù ci si può muovere tramite i quattro pulsanti:
 *          - up: scorri in su il selettore
 *          - down: scorri in giù il selettore
 *          - right: entri nel sotto menù se disponibile indicato dal selettore o salvare le impostazioni. dopo aver salvati vieni riportato al menù precedente in automatico
 *          - left: esci dal sotto menù e nel caso in cui ci siano delle moficihe alle impostazioni non vengono salvate
 * 
 *      In particolare i sotto menù sono composti in questo modo
 *          - data extraction:
 *              schermata di avanzamento dell'acquisione dati =>  es. 100/25000
 *              (volendo si potrebbe mettere l'ora attuale presa dal gps)
 *          - usb:
 *              selezione se scaricare o no i dati tramite selettore up/down => es. USB mass storage Enable/Disabled
 *          - settings:
 *              sotto menù con:
 *                  - temperatura:
 *                      - sampling rate (numero di acquisizione per ora?)
 *                  - accelrometro:
 *                      - sampling rate (numero di acquisizione per ora?)
 *                      - quali assi salvare:
 *                          - x => activate/disactivate
 *                          - y => activate/disactivate
 *                          - z => activate/disactivate
 *                  - gps:
 *                      - sampling rate (numero di acquisizione per ora?)
 *                      - quali valori da salvare:
 *                          - ora => alwalys active
 *                          - da specificare
 *                  
 *      addictional features:
 *          - emettere il rumore del buzzer ogni volta che si clicca un bottone
 *          - emmettere un rumore diverso se il punsante cliccato non porta a nessun cambianto
 *          - emettere due suoni diversi quando si salva e si esce o si esce e basta dalle impostazioni
 * 
 * 
 * 
// livello di profondità di visualizzazione/ modifica, più è alto il valore più si è profondi
// profodità 0 = scelta tra settings, data extraction e usb download
// profondità 1 = si entra nei vari sotto menù (usb enable/disable) + (data extraction info) + (settings temp/acc/gps)
// profondità 2 = è possibile raggiungerlo solo dentro settings e si sceglie l'attributo da modificare
// profondità 3 = è possibile raggiungerlo solo dentro settings e si sceglie il valore dell'attributo da modificare

****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "pico.h"
#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"

//not use now: uncomment and add to CMakeList if needed
//#include "hardware/rtc.h"
//#include "hardware/pll.h"
//#include "hardware/xosc.h"
//#include "hardware/rosc.h"
//#include "hardware/regs/io_bank0.h"


// #include "pico/sleep.h"

// For __wfi
#include "hardware/sync.h"
// For scb_hw so we can enable deep sleep
#include "hardware/structs/scb.h"

#include "datatypes.h"
#include "accelerometer.h"
#include "temperature.h"
#include "gps.h"
#include "buzzer.h"
#include "screen.h"
#include "led.h"
#include "FSMtypes.h"

#include "pitches.h"

#include "cdc_msc/main.c"


#define MAX_SAMPLING_RATE 60*60 // 1hr


static bool started = false;
bool interrompi = false;
bool exit_usb_mode = false;

/*******************************************************************/
/* definizioni delle funzioni di inizializzazione */

void buttons_callback(uint gpio, uint32_t events);

void init_enviroment(){
    stdio_init_all();
    hw_clear_bits(&timer_hw->pause, TIMER_PAUSE_BITS);
}
void init_buttons(){
    gpio_pull_up(22);
    gpio_pull_up(23);
    gpio_pull_up(24);
    gpio_pull_up(25);
    gpio_set_irq_enabled_with_callback(22, GPIO_IRQ_LEVEL_LOW, true, &buttons_callback);
    gpio_set_irq_enabled_with_callback(23, GPIO_IRQ_LEVEL_LOW, true, &buttons_callback);
    gpio_set_irq_enabled_with_callback(24, GPIO_IRQ_LEVEL_LOW, true, &buttons_callback);
    gpio_set_irq_enabled_with_callback(25, GPIO_IRQ_LEVEL_LOW, true, &buttons_callback);
    // per cambio stato GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL
}

void init_hardware(){
    init_enviroment();
    init_buttons();
    init_screen();
    init_temperature();
    init_gps();
    init_accelerometer();
    init_buzzer();
    init_led();
}

/*******************************************************************/
/* definizioni delle funzioni per l'acquisizione dati */

void init_Data_storage(struct Data_storage *Data){
    Data->Hours  = 0;
    Data->Minutes  = 0;
    Data->Seconds  = 0;
    Data->Longitude  = 0.0;
    Data->Latitude  = 0.0;
    Data->Altitude  = 0;
    Data->x_acceleration  = 0;
    Data->y_acceleration  = 0;
    Data->z_acceleration  = 0;
    Data->temperature  = 0;
}

void print_DATA(struct Data_storage *Data){
    printf("TIME: %d:%d:%d \n", Data->Hours, Data->Minutes, Data->Seconds);
    printf("POSITION: Longitude = %0.2f, \t Latitude = %0.2f, \t Altitude = %0.2f \n", Data->Longitude, Data->Latitude, Data->Altitude);
    printf("ACCELEROMETER: X = %d, \t X = %d, \t X = %d \n", Data->x_acceleration, Data->y_acceleration, Data->z_acceleration);
    printf("TEMPERATURE: %d Celsius \n", Data->temperature);
}

/*******************************************************************/
/* state - functioncall link */

void fn_INIT();
void fn_DEFAULT();

void fn_SETTINGS();
void fn_DATA_EXTRACTION();
void fn_USB();

void fn_TEMPERATURA();
void fn_ACCELEROMETRO();
void fn_GPS();
void fn_SAMPLING_RATE();

void fn_ACC_X();
void fn_ACC_Y();
void fn_ACC_Z();
void fn_ACC_SAMPLING_RATE();

void fn_GRADI_1_GPS();
void fn_GRADI_2_GPS();
void fn_ALTITUDINE_GPS();

StateMachine_Menu_t fsm_INIT_DEFAULT[] = {
    {STATE_INIT, fn_INIT},
    {STATE_DEFAULT,fn_DEFAULT}
};

StateMachine_Function_t fsm_SETTINGS__DATA_EXTRACTIOn_USB[] = {
                      {STATE_SETTINGS, fn_SETTINGS},
                      {STATE_DATA_EXTRACTION, fn_DATA_EXTRACTION},
                      {STATE_USB, fn_USB}
};

StateMachine_Settings_t fsm_SETTINGS[] = {
                      {STATE_TEMPERATURA, fn_TEMPERATURA},
                      {STATE_ACCELEROMETRO, fn_ACCELEROMETRO},
                      {STATE_GPS, fn_GPS},
                      {STATE_SAMPLING_RATE, fn_SAMPLING_RATE},

};

StateMachine_Accelerometer_t fsm_Accelerometer[] = {
                      {STATE_X_ACC, fn_ACC_X},
                      {STATE_Y_ACC, fn_ACC_Y},
                      {STATE_Z_ACC, fn_ACC_Z},
};

StateMachine_GPS_t fsm_GPS[] = {
                      {GRADI_1_GPS,fn_GRADI_1_GPS},
                      {GRADI_2_GPS,fn_GRADI_2_GPS},
                      {ALTITUDINE_GPS,fn_ALTITUDINE_GPS}
};


/*******************************************************************/
/* menu state */

void fn_INIT(){
    // inizializziamo tutti i vari parametri come default
    init_hardware();
    clear_screen();
    WriteString_Fonts_8x8(4,0,"initialization");
    WriteString_Fonts_12x16(30,50,"done");
    printf("initialization done \n");
    settings.current_state_menu = STATE_DEFAULT;
    tmp_settings = settings;
    interrompi = true;
}

void fn_DEFAULT(){
    if(settings.depth > 0 ){
    // è lo stato di selezione di una delle tre modalità sottostanti
        if(settings.current_state_function < NUM_STATES_FUNCTION){
            (*fsm_SETTINGS__DATA_EXTRACTIOn_USB[settings.current_state_function].state_function)();     //call to FSM functions
        }
        else{
            /* error */
        }
    }
    else{
        uint8_t leds_color[8*3];
        int i = 0;
        tmp_settings = settings;
        printf("USB, DATA EXTRACTION e SETTINGS \n");
        printf("selettore su:");
        switch (settings.current_state_function)
        {
        case STATE_SETTINGS:
            printf("\t STATE_SETTINGS \n");
            clear_screen();
            write_menu_on_screen("MENU'","Start Sampling","Settings","");
            break;
        case STATE_DATA_EXTRACTION:

            for(i=0;i<8;i++){
                leds_color[0 + i*3] = 0;
                leds_color[1 + i*3] = 0;
                leds_color[2 + i*3] = 0;
            }
            set_sequential_led(leds_color,8);
            printf("\t STATE_DATA_EXTRACTION \n");
            started = false;
            clear_screen();
            write_menu_on_screen("MENU'","USB transfer","Start Sampling","Settings");
            break;
        case STATE_USB:
            printf("\t STATE_USB \n");
            clear_screen();
            write_menu_on_screen("MENU'","","USB transfer","Start Sampling");
            break;
        }

        // mostra a schermo la selezione di uno dei tre stati
    }
 
}

/*******************************************************************/
/* function state */

void fn_SETTINGS(){
    //default value
        //data extraction pause => disable all the interrupts etc
        // USB.trasnfer = false

    //gestisce tutte le varie cose di STATE_SETTINGS
    if(settings.depth > 1){
        // è lo stato di modifica dei settings
        if(settings.current_state_settings < NUM_STATES_SETTINGS){
            (*fsm_SETTINGS[settings.current_state_settings].state_function)();     //call to FSM functions
        }
        else{
            /* error */
        }
    }
    else{
        tmp_settings = settings;
        printf("ACC, TEMP e GPS \n");
        printf("selettore su:");
        switch (settings.current_state_settings)
        {
        case STATE_TEMPERATURA:
            printf("\t STATE_TEMPERATURA \n");
            clear_screen();
            write_menu_on_screen("SETTINGS","Accelerometer","Temperature","");
            break;
        case STATE_ACCELEROMETRO:
            printf("\t STATE_ACCELEROMETRO \n");
            clear_screen();
            write_menu_on_screen("SETTINGS","GPS","Accelerometer","Temperature");
            break;
        case STATE_GPS:
            printf("\t STATE_GPS \n");
            clear_screen();
            write_menu_on_screen("SETTINGS","Sampling rate","GPS","Accelerometer");
            break;
        case STATE_SAMPLING_RATE:
            printf("\t STATE_SAMPLING_RATE \n");
            clear_screen();
            write_menu_on_screen("SETTINGS","","Sampling rate","GPS");
            break;
        } 
        // mostra a schermo la selezione tra acc/temp/GPS
    }


}

void fn_DATA_EXTRACTION(){
    //default values for this state
        // USB.trasnfer = false
    settings.final_depth = true;
    //gestisce tutte le varie cose di STATE_DATA_EXTRACTION
    if(settings.depth > 1){
        // errore, DATA_EXTRACTION non ha questa profondità
    }else{
        static int iteration = 0;
        struct Data_storage Data;
        static uint64_t time;
        static uint64_t time_last_call;
        if(!started){
            printf("DATA EXTRACTION\n");
            printf("more info \n");
            init_Data_storage(&Data);
            iteration = 0;
            clear_screen();
            write_menu_on_screen("SAMPLING","","loading","");
            init_file_string();
            strcat(README_CONTENTS, "DATA, ORA, LATITUDINE, LONGITUDINE, ALTITUDINE, ACCELEROMETRO X, ACCELEROMETRO Y, ACCELEROMETRO Z, TEMPERATURA \r\n\r\n");
            time_last_call = time_us_64();
        }

        while(!interrompi && iteration != 99){
            started = true;
            get_temperature(&Data);
            get_accelerometer(&Data);
            get_gps(&Data);
            iteration++;
            char  string_1 [10]; 
            sprintf(string_1," %d",iteration);
            char string_2 [10];
            sprintf(string_2,"/100");
            clear_screen();
            write_menu_on_screen("SAMPLING","iteration:",string_1,string_2);
            print_DATA(&Data);
            char str_to_usb [120] = "";
            sprintf(str_to_usb,"%f, %d:%d:%d, %0.5fN, %0.5fE, %0.1fm, %0.2fm/s, %0.2fm/s,  %0.2fm/s, %0.2fC \r\n");
            strcat(README_CONTENTS, str_to_usb);
            // da modificare un po' intricato
            time = time_us_64();
            while (time - time_last_call  < (settings.Sampling_Rate*1000000) )
            {
                uint8_t leds_color[8*3];
                int i = 0;
                for(i=0;i<8;i++){
                    leds_color[0 + i*3] = 0;
                    leds_color[1 + i*3] = 0;
                    leds_color[2 + i*3] = 0;
                }
                if(time - time_last_call  < 1*(settings.Sampling_Rate*1000000)/9){
                    for(i=0;i<8;i++){
                        leds_color[0 + i*3] = 0;
                        leds_color[1 + i*3] = 0;
                        leds_color[2 + i*3] = 0;
                    }
                }
                else if(time - time_last_call  < 2*(settings.Sampling_Rate*1000000)/9){
                    for(i=0;i<1;i++){
                        leds_color[0 + i*3] = 255;
                        leds_color[1 + i*3] = 255;
                        leds_color[2 + i*3] = 255;
                    }
                }
                else if (time - time_last_call  < 3*(settings.Sampling_Rate*1000000)/9)
                {
                    for(i=0;i<2;i++){
                        leds_color[0 + i*3] = 255;
                        leds_color[1 + i*3] = 255;
                        leds_color[2 + i*3] = 255;
                    }                }
                else if (time - time_last_call  < 4*(settings.Sampling_Rate*1000000)/9)
                {
                    for(i=0;i<3;i++){
                        leds_color[0 + i*3] = 255;
                        leds_color[1 + i*3] = 255;
                        leds_color[2 + i*3] = 255;
                    }                }
                else if (time - time_last_call  < 5*(settings.Sampling_Rate*1000000)/9)
                {
                    for(i=0;i<4;i++){
                        leds_color[0 + i*3] = 255;
                        leds_color[1 + i*3] = 255;
                        leds_color[2 + i*3] = 255;
                    }                }
                else if (time - time_last_call  < 6*(settings.Sampling_Rate*1000000)/9)
                {
                    for(i=0;i<5;i++){
                        leds_color[0 + i*3] = 255;
                        leds_color[1 + i*3] = 255;
                        leds_color[2 + i*3] = 255;
                    }                }
                else if (time - time_last_call  < 7*(settings.Sampling_Rate*1000000)/9)
                {
                    for(i=0;i<6;i++){
                        leds_color[0 + i*3] = 255;
                        leds_color[1 + i*3] = 255;
                        leds_color[2 + i*3] = 255;
                    }                }
                else if (time - time_last_call  < 7*(settings.Sampling_Rate*1000000)/8){
                    for(i=0;i<7;i++){
                        leds_color[0 + i*3] = 255;
                        leds_color[1 + i*3] = 255;
                        leds_color[2 + i*3] = 255;
                    }                
                }
                else{
                    for(i=0;i<8;i++){
                        leds_color[0 + i*3] = 255;
                        leds_color[1 + i*3] = 255;
                        leds_color[2 + i*3] = 255;
                    }                     
                }
                set_sequential_led(leds_color,8);

                if(interrompi){
                    for(i=0;i<8;i++){
                        leds_color[0 + i*3] = 0;
                        leds_color[1 + i*3] = 0;
                        leds_color[2 + i*3] = 0;
                    }
                    set_sequential_led(leds_color,8);
                    break;
                }
                time = time_us_64();
            }
            time_last_call = time_last_call + (settings.Sampling_Rate*1000000);


        }

        if(!interrompi){
            int i = 0;
            uint8_t leds_color[8*3];
            clear_screen();
            write_menu_on_screen("SAMPLING","iteration:","DONE","");
            end_acquisition();
            busy_wait_ms(500);
            for(i=0;i<8;i++){
                leds_color[0 + i*3] = 255;
                leds_color[1 + i*3] = 255;
                leds_color[2 + i*3] = 255;
            }
            busy_wait_ms(1000);
            for(i=0;i<8;i++){
                leds_color[0 + i*3] = 0;
                leds_color[1 + i*3] = 0;
                leds_color[2 + i*3] = 0;
            }
                        busy_wait_ms(500);
            for(i=0;i<8;i++){
                leds_color[0 + i*3] = 255;
                leds_color[1 + i*3] = 255;
                leds_color[2 + i*3] = 255;
            }
            busy_wait_ms(1000);
            for(i=0;i<8;i++){
                leds_color[0 + i*3] = 0;
                leds_color[1 + i*3] = 0;
                leds_color[2 + i*3] = 0;
            }
            busy_wait_ms(500);
            for(i=0;i<8;i++){
                leds_color[0 + i*3] = 255;
                leds_color[1 + i*3] = 255;
                leds_color[2 + i*3] = 255;
            }
            busy_wait_ms(1000);
            for(i=0;i<8;i++){
                leds_color[0 + i*3] = 0;
                leds_color[1 + i*3] = 0;
                leds_color[2 + i*3] = 0;
            }
            set_sequential_led(leds_color,8);
        }
    }

}
void fn_USB(){
    //default value
        //data extraction pause => disable all the interrupts etc

    settings.final_depth = true;
    //gestisce tutte le varie cose di STATE_USB
    if(settings.depth > 1){
        // errore, USB non ha questa profondità
    }
    else{
        printf("USB \n");
        printf("more info \n");
        clear_screen();
        write_menu_on_screen("USB","","transfer","");
        init_file_user_define();
        init_USB();
        tud_connect();
        while(1){
            tud_connect();
            USB_transfer();
            printf("USB tranfer \n");
            if(exit_usb_mode){
                tud_disconnect();
                interrompi = true;
                exit_usb_mode = false;
                break;
            }
        }
 
        // mostra a schermo enable USB transfer while you are in this menù
        // da aggiungere che quando si esce da questo selezione l'usb tasnfer si blocca
    }
}


/*******************************************************************/
/* settings state */

void fn_TEMPERATURA(){
    settings.final_depth = true;
    printf("TEMPERATURA \n");
    printf("more info:\n");
    printf("enable:  %d \n",tmp_settings.Temp);
    if(tmp_settings.Temp){
        clear_screen();
        write_menu_on_screen("TEMP","","Enabled","Disabled");
    }
    else{
        clear_screen();
        write_menu_on_screen("TEMP","Enabled","Disabled","");
    }
    //gestisce tutte le varie impostazioni di TEMPERATURA
    // mostrare a schermo
    // TITOLO: SAMPLING RATE
    // sottotitolo: valore temporaneo dei SAMPLING RATE
}

void fn_ACCELEROMETRO(){
    //gestisce tutte le varie impostazioni di ACCELEROMETRO
    if(settings.depth > 2){
        if(settings.current_state_accelerometer < NUM_STATES_ACCELEROMETER){
            (*fsm_Accelerometer[settings.current_state_accelerometer].state_function)();     //call to FSM functions
        }
        else{
            /* error */
        }
    }
    else{
        tmp_settings = settings;
        printf("ACC: X, Y, Z \n");
        printf("selettore su:");
        switch (settings.current_state_accelerometer)
        {
        case STATE_X_ACC:
            printf("\t STATE_X_ACC \n");
            clear_screen();
            write_menu_on_screen("ACC:","Y","X","");
            break;
        case STATE_Y_ACC:
            printf("\t STATE_Z_ACC \n");
            clear_screen();
            write_menu_on_screen("ACC:","Z","Y","X");
            break;
        case STATE_Z_ACC:
            printf("\t STATE_Z_ACC \n");
            clear_screen();
            write_menu_on_screen("ACC:","","Z","Y");
            break;
        }
        // mostrare a schermo il menù con i vari attributi
    }
}

void fn_GPS(){
    //gestisce tutte le varie impostazioni di GPS
    if(settings.depth > 2){
        if(settings.current_state_GPS < NUM_STATES_GPS){
            (*fsm_GPS[settings.current_state_GPS].state_function)();     //call to FSM functions
        }
        else{
            /* error */
        }
    }
    else{
        tmp_settings = settings;
        printf("GPS: sampling rate \n");
        printf("selettore su:");
        switch (settings.current_state_GPS)
        {
        case GRADI_1_GPS:
            printf("\t case GRADI_1_GPS \n");
            clear_screen();
            write_menu_on_screen("GPS:","LON","LAT","");
            break;
        case GRADI_2_GPS:
            printf("\t case GRADI_2_GPS \n");
            clear_screen();
            write_menu_on_screen("GPS:","ALT","LON","LAT");
            break;
        case ALTITUDINE_GPS:
            printf("\t case ALTITUDINE_GPS \n");
            clear_screen();
            write_menu_on_screen("GPS:","","ALT","LON");
            break;
        }
        // mostrare a schermo il menù con i vari attributi
    }
}

void fn_SAMPLING_RATE(){
    settings.final_depth = true;
    printf("SAMPLING_RATE \n");
    printf("more info:\n");
    printf("sampling rate:  %d  \n",tmp_settings.Sampling_Rate);
    char string_1 [10];
    sprintf(string_1,"%d s",tmp_settings.Sampling_Rate);
    clear_screen();
    write_menu_on_screen("SAMPLING","sampling every:", string_1, " max = 3600");
    // mostra a schermo il valore temporaneo di SAMPLING_RATE
}


/*******************************************************************/
/* state GPS - attribute */

void fn_GRADI_1_GPS(){
    settings.final_depth = true;
    printf("GPS - GRADI_1 \n");
    printf("more info:\n");
    printf("sampling rate:  %d \n",tmp_settings.Gradi_1_GPS);
    if(tmp_settings.Gradi_1_GPS){
        clear_screen();
        write_menu_on_screen("GPS - LAT","","Enabled","Disabled");
    }
    else{
        clear_screen();
        write_menu_on_screen("GPS - LAT","Enabled","Disabled","");
    }
    // mostra a schermo il valore temporaneo di SAMPLING_RATE
}
void fn_GRADI_2_GPS(){
    settings.final_depth = true;
    printf("GPS - GRADI_2 \n");
    printf("more info:\n");
    printf("sampling rate:  %d \n",tmp_settings.Gradi_2_GPS);
    if(tmp_settings.Gradi_2_GPS){
        clear_screen();
        write_menu_on_screen("GPS - LON","","Enabled","Disabled");
    }
    else{
        clear_screen();
        write_menu_on_screen("GPS - LON","Enabled","Disabled","");
    }
    // mostra a schermo il valore temporaneo di SAMPLING_RATE
}
void fn_ALTITUDINE_GPS(){
    settings.final_depth = true;
    printf("GPS - ALTIUDINE \n");
    printf("more info:\n");
    printf("sampling rate:  %d \n",tmp_settings.Altitudine_GPS);
    if(tmp_settings.Altitudine_GPS){
        clear_screen();
        write_menu_on_screen("GPS - ALT","","Enabled","Disabled");
    }
    else{
        clear_screen();
        write_menu_on_screen("GPS - ALT","Enabled","Disabled","");
    }
    // mostra a schermo il valore temporaneo di SAMPLING_RATE
}

/*******************************************************************/
/* state ACC - attribute */

void fn_ACC_X(){
    settings.final_depth = true;
    printf("ACCELEROMETER \n");
    printf("more info:\n");
    printf("X(=0 if not enabled, =1 if enabled):  %d  \n",tmp_settings.Acc_X);
    if(tmp_settings.Acc_X){
        clear_screen();
        write_menu_on_screen("ACC - X ","","Enabled","Disabled");
    }
    else{
        clear_screen();
        write_menu_on_screen("ACC - X ","Enabled","Disabled","");
    }
    // mostra a schermo il valore temporaneo di X (ENABLE OR DISABLE)
}
void fn_ACC_Y(){
    settings.final_depth = true;
    printf("ACCELEROMETER \n");
    printf("more info:\n");
    printf("Y(=0 if not enabled, =1 if enabled):  %d \n",tmp_settings.Acc_Y);
    if(tmp_settings.Acc_Y){
        clear_screen();
        write_menu_on_screen("ACC - Y ","","Enabled","Disabled");
    }
    else{
        clear_screen();
        write_menu_on_screen("ACC - Y ","Enabled","Disabled","");
    }
    // mostra a schermo il valore temporaneo di y (ENABLE OR DISABLE)
}
void fn_ACC_Z(){
    settings.final_depth = true;
    printf("ACCELEROMETER \n");
    printf("more info:\n");
    printf("Z(=0 if not enabled, =1 if enabled):  %d \n",tmp_settings.Acc_Z);
    if(tmp_settings.Acc_Z){
        clear_screen();
        write_menu_on_screen("ACC - Z ","","Enabled","Disabled");
    }
    else{
        clear_screen();
        write_menu_on_screen("ACC - Z ","Enabled","Disabled","");
    }
    // mostra a schermo il valore temporaneo di z (ENABLE OR DISABLE)
}


/*******************************************************************/
/* button functions */

void left_button(){
    if(settings.final_depth){
        exit_nosave();
        tmp_settings = settings;
        settings.final_depth = false;
    }

    if(!settings.start_depth){
        click_button();

        settings.depth--;
        if(settings.depth == 0){
            settings.start_depth = true;
        }
    }
    else{
        end_menu();
    }

}

void right_button(){
    // printf("%d", settings.final_depth);
    settings.start_depth = false;
    if(settings.final_depth){
        exit_save();
        settings.depth--;
        settings = tmp_settings;
        settings.final_depth = false;
    }
    else{
        click_button();
        settings.depth++;
    }
}

void up_button(){
    if(!settings.final_depth){
        switch (settings.depth){
            case 0:
                if(!(settings.current_state_function == NUM_STATES_FUNCTION-1)){
                    settings.current_state_function ++;
                    click_button();
                }
                else {
                    end_menu();
                }
                break;
            case 1:
                if(!(settings.current_state_settings == NUM_STATES_SETTINGS-1)){
                    settings.current_state_settings ++;
                    click_button();
                }
                else {
                    end_menu();
                }
                break;
            case 2:
                switch(settings.current_state_settings){
                    case STATE_ACCELEROMETRO:
                        if(!(settings.current_state_accelerometer == NUM_STATES_ACCELEROMETER-1)){
                            settings.current_state_accelerometer ++;
                            click_button();
                        }
                        else {
                            end_menu();
                        }
                        break;
                    case STATE_GPS:
                        if(!(settings.current_state_GPS == NUM_STATES_ACCELEROMETER-1)){
                            settings.current_state_GPS ++;
                            click_button();
                        }
                        else {
                            end_menu();
                        }
                        break;
                }
                break;
            case 3:
                //pass
                break;
        }
    }
        //modifica la variable a schermo
        // tmp_settings.VARIABILE UP/++/toggle

    else
    {
       switch (settings.current_state_function){
            case STATE_DATA_EXTRACTION:
            case STATE_USB:
            break;
            case STATE_SETTINGS:
                switch (settings.current_state_settings){
                    case STATE_TEMPERATURA:
                        tmp_settings.Temp = 1 - tmp_settings.Temp;
                    break;
                    case STATE_ACCELEROMETRO:
                        switch (settings.current_state_accelerometer){
                            case STATE_X_ACC:
                                tmp_settings.Acc_X = 1 - tmp_settings.Acc_X;
                                break;
                            case STATE_Y_ACC:
                                tmp_settings.Acc_Y = 1 - tmp_settings.Acc_Y;
                                break;
                            case STATE_Z_ACC:
                                tmp_settings.Acc_Z = 1 - tmp_settings.Acc_Z;
                                break;                          
                        }
                    break;
                    case STATE_GPS:
                        switch (settings.current_state_GPS){
                            case GRADI_1_GPS:
                                tmp_settings.Gradi_1_GPS = 1 - tmp_settings.Gradi_1_GPS;
                                break;
                            case GRADI_2_GPS:
                                tmp_settings.Gradi_2_GPS = 1 - tmp_settings.Gradi_2_GPS;
                                break;
                            case ALTITUDINE_GPS:
                                tmp_settings.Altitudine_GPS = 1 - tmp_settings.Altitudine_GPS;
                                break;                        
                        }                 
                        break;
                    case STATE_SAMPLING_RATE:
                        if(tmp_settings.Temp < MAX_SAMPLING_RATE){
                            tmp_settings.Sampling_Rate++;
                                click_button();
                        }
                        else {
                            end_menu();
                        }
                        break;
                    }
            break;
        }
    }

}

void down_button(){
    if(!settings.final_depth){
        switch (settings.depth){
            case 0:
                if(!(settings.current_state_function == 0)){
                    settings.current_state_function --;
                    click_button();
                }
                else {
                    end_menu();
                }
                break;
            case 1:
                if(!(settings.current_state_settings == 0)){
                    settings.current_state_settings --;
                    click_button();
                }
                else {
                    end_menu();
                }
                break;
            case 2:
                switch (settings.current_state_settings){
                case STATE_ACCELEROMETRO:
                    if(!(settings.current_state_accelerometer == 0)){
                        settings.current_state_accelerometer --;
                        click_button();
                    }
                    else {
                        end_menu();
                    }
                    break;
                case STATE_GPS:
                    if(!(settings.current_state_GPS == 0)){
                        settings.current_state_GPS --;
                        click_button();
                    }
                    else {
                        end_menu();
                    }
                    break;
                }

                break;
            case 3:
                //pass
                break;
        }
    }
    
        //modifica la variable a schermo
        // tmp_settings.VARIABILE DOWN/--/toggle
    else
    {
       switch (settings.current_state_function){
            case STATE_DATA_EXTRACTION:
            case STATE_USB:
            break;
            case STATE_SETTINGS:
                switch (settings.current_state_settings){
                    case STATE_TEMPERATURA:
                        tmp_settings.Temp = 1 - tmp_settings.Temp;
                    break;
                    case STATE_ACCELEROMETRO:
                        switch (settings.current_state_accelerometer){
                            case STATE_X_ACC:
                                tmp_settings.Acc_X = 1 - tmp_settings.Acc_X;
                                break;
                            case STATE_Y_ACC:
                                tmp_settings.Acc_Y = 1 - tmp_settings.Acc_Y;
                                break;
                            case STATE_Z_ACC:
                                tmp_settings.Acc_Z = 1 - tmp_settings.Acc_Z;
                                break;                          
                        }
                        break;
                    case STATE_GPS:
                        switch (settings.current_state_GPS){
                            case GRADI_1_GPS:
                                tmp_settings.Gradi_1_GPS = 1 - tmp_settings.Gradi_1_GPS;
                                break;
                            case GRADI_2_GPS:
                                tmp_settings.Gradi_2_GPS = 1 - tmp_settings.Gradi_2_GPS;
                                break;
                            case ALTITUDINE_GPS:
                                tmp_settings.Altitudine_GPS = 1 - tmp_settings.Altitudine_GPS;
                                break;                        
                        }                 
                        break;
                    case STATE_SAMPLING_RATE:
                        if(tmp_settings.Temp > 2){
                            tmp_settings.Sampling_Rate--;
                            click_button();
                            
                        }
                        else {
                            end_menu();
                        }
                        break;
                }
            break;
        }
    }    
}


/*******************************************************************/
/* sleep mode */

void sleep_mode() {
    // We should have already called the sleep_run_from_dormant_source function
    // assert(dormant_source_valid(_dormant_source));
    // Turn off all clocks when in sleep mode except for RTC
    clocks_hw->sleep_en0 = CLOCKS_SLEEP_EN0_CLK_RTC_RTC_BITS;
    clocks_hw->sleep_en1 = 0x0;
    uint save = scb_hw->scr;
    // Enable deep sleep at the proc
    scb_hw->scr = save | M0PLUS_SCR_SLEEPDEEP_BITS;
    // Go to sleep
    __wfi();
}

uint64_t get_time(void) {
    // Reading low latches the high value
    uint32_t lo = timer_hw->timelr;
    uint32_t hi = timer_hw->timehr;
    return ((uint64_t) hi << 32u) | lo;
}


/*******************************************************************/
/* interrput handler */

#define time_interval_between_button_push 300000 
static uint64_t time_bt_up = 0;
static uint64_t time_bt_down = 0;
static uint64_t time_bt_right = 0;
static uint64_t time_bt_left = 0;

void buttons_callback(uint gpio, uint32_t events) {
    uint64_t time = time_us_64();
    uint64_t difference = 100000;
    switch (gpio)
    {
    case 22:
        difference = time - time_bt_up;
        if(difference > time_interval_between_button_push){
            exit_usb_mode = true;
            interrompi = true;
            time_bt_up = time;
            up_button();
            // printf("clicked up \n");
        }
        break;
    case 23:
        difference = time - time_bt_down;
        if(difference > time_interval_between_button_push){
            exit_usb_mode = true;
            interrompi = true;
            time_bt_down = time;
            down_button();
            // printf("clicked down \n");
        }
        break;
    case 24:
        difference = time - time_bt_right;
        if(difference > time_interval_between_button_push){
            exit_usb_mode = true;
            interrompi = true;
            time_bt_right = time;
            right_button();
            // printf("clicked right \n");
        }
        break;
    case 25:
        difference = time - time_bt_left;
        if(difference > time_interval_between_button_push){
            exit_usb_mode = true;
            interrompi = true;
            time_bt_left = time;
            left_button();
            // printf("clicked left \n");
        }
        break;
    default:
        break;
    }
    // printf("GPIO %d\n", gpio);
}


int main() {
    //lasciamo che l'applicazione interrupt based faccia il suo corso
    while(1)
    {
        if(settings.current_state_menu < NUM_STATES_MENU){
            (*fsm_INIT_DEFAULT[settings.current_state_menu].state_function)();     //call to FSM functions
        }
        else{
            /* error */
        }
        while(1){
            if(interrompi == true){
                interrompi = false;
                break;
            }
        }
    }
    
}
