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
#include "pico/stdlib.h"

// #include "pico/sleep.h"

#include "pico.h"
#include "hardware/gpio.h"


#include "hardware/rtc.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/xosc.h"
#include "hardware/rosc.h"
#include "hardware/regs/io_bank0.h"
// For __wfi
#include "hardware/sync.h"
// For scb_hw so we can enable deep sleep
#include "hardware/structs/scb.h"

// #include "hardware/gpio.h"
// #include "hardware/adc.h"

#define MAX_SAMPLING_RATE 150
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
void int_screen(){
    
}
void init_adc(){
    
}
void init_gps(){
    
}
void init_accelerometer(){
    
}
void init_buzzer(){

}

void init_led(){

}
void init_hardware(){
    init_enviroment();
    init_buttons();
    int_screen();
    init_adc();
    init_gps();
    init_accelerometer();
    init_buzzer();
    init_led();
}

/******************************************************************
* definition of the Finite State Machine structure */

typedef enum{
    STATE_INIT,
    STATE_DEFAULT,
    NUM_STATES_MENU
}State_Menu;

typedef struct{
    State_Menu state;
    void (*state_function)(void);
} StateMachine_Menu_t;


typedef enum {
    STATE_SETTINGS,
    STATE_DATA_EXTRACTION,
    STATE_USB,
    NUM_STATES_FUNCTION
}State_Function;

typedef struct{
    State_Function state;
    void (*state_function)(void);
} StateMachine_Function_t;


typedef enum {
    STATE_TEMPERATURA,
    STATE_ACCELEROMETRO,
    STATE_GPS,
    NUM_STATES_SETTINGS
}State_Settings;

typedef struct{
    State_Settings state;
    void (*state_function)(void);
} StateMachine_Settings_t;

typedef enum {
    STATE_X_ACC,
    STATE_Y_ACC,
    STATE_Z_ACC,
    STATE_SAMPLING_RATE_ACC,
    NUM_STATES_ACCELEROMETER
}State_Accelerometer;

typedef struct{
    State_Accelerometer state;
    void (*state_function)(void);
} StateMachine_Accelerometer_t;

typedef enum {
    // add features
    STATE_SAMPLING_RATE_GPS,
    NUM_STATES_GPS
}State_GPS;

typedef struct{
    State_GPS state;
    void (*state_function)(void);
} StateMachine_GPS_t;

/*******************************************************************/
/* Define struct as data storage of all settings */
typedef struct {
    /* variables to hold current state */
    State_Menu current_state_menu;
    State_Function current_state_function;
    State_Settings current_state_settings;
    State_Accelerometer current_state_accelerometer;
    State_GPS current_state_GPS;

    bool start_depth;
    int depth;
    bool final_depth;

    // accelerometer
    bool Acc_X;
    bool Acc_Y;
    bool Acc_Z;
    int Acc_Sampling_Rate;

    // temperatura
    int Temp_Sampling_Rate;

    // gps
    int GPS_Sampling_Rate;




    /* data */
}Settings_storage;

///////////////////////////////   definizioni delle variabili globali    ////////////////////////////////////////////



Settings_storage settings = {
    
    
    
    
    
    STATE_INIT, // State_Menu current_state_menu;
    STATE_SETTINGS, // State_Function current_state_function;
    STATE_TEMPERATURA, // State_Settings current_state_settings;
    STATE_SAMPLING_RATE_ACC, // State_Accelerometer current_state_accelerometer;
    STATE_SAMPLING_RATE_GPS, // State_GPS current_state_GPS;
    
    true, // bool start_depth;
    0, // int depth;
    false, // bool final_depth;

    // // accelerometer
    true, // bool Acc_X;
    true, // bool Acc_Y;
    true, // bool Acc_Z;
    100, // int Acc_Sampling_Rate;

    // // temperatura
    100, // int Temp_Sampling_Rate;

    // // gps
    100 // int GPS_Sampling_Rate;
    

};

Settings_storage tmp_settings = {

};

///////////////////////////////   state - functioncall link    ////////////////////////////////////////////
void fn_INIT();
void fn_DEFAULT();

void fn_SETTINGS();
void fn_DATA_EXTRACTION();
void fn_USB();

void fn_TEMPERATURA();
void fn_ACCELEROMETRO();
void fn_GPS();

void fn_ACC_X();
void fn_ACC_Y();
void fn_ACC_Z();
void fn_ACC_SAMPLING_RATE();

void fn_GPS_SAMPLING_RATE();


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
                      {STATE_GPS, fn_GPS}
};

StateMachine_Accelerometer_t fsm_Accelerometer[] = {
                      {STATE_X_ACC, fn_ACC_X},
                      {STATE_Y_ACC, fn_ACC_Y},
                      {STATE_Z_ACC, fn_ACC_Z},
                      {STATE_SAMPLING_RATE_ACC, fn_ACC_SAMPLING_RATE }
};

StateMachine_GPS_t fsm_GPS[] = {
                      {STATE_SAMPLING_RATE_GPS, fn_GPS_SAMPLING_RATE},
};


///////////////////////////////   menu state    ////////////////////////////////////////////

void fn_INIT(){
    // inizializziamo tutti i vari parametri come default
    init_hardware();
    printf("initialization done \n");
    settings.current_state_menu = STATE_DEFAULT;
    tmp_settings = settings;

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
        tmp_settings = settings;
        printf("USB, DATA EXTRACTION e SETTINGS \n");
        printf("selettore su:");
        switch (settings.current_state_function)
        {
        case STATE_SETTINGS:
            printf("\t STATE_SETTINGS \n");
            break;
        case STATE_DATA_EXTRACTION:
            printf("\t STATE_DATA_EXTRACTION \n");
            break;
        case STATE_USB:
            printf("\t STATE_USB \n");
            break;
        }

        // mostra a schermo la selezione di uno dei tre stati
    }
 
}

///////////////////////////////   function state    ////////////////////////////////////////////

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
        case STATE_ACCELEROMETRO:
            printf("\t STATE_ACCELEROMETRO \n");
            break;
        case STATE_TEMPERATURA:
            printf("\t STATE_TEMPERATURA \n");
            break;
        case STATE_GPS:
            printf("\t STATE_GPS \n");
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
    }
    else{

        printf("DATA EXTRACTION\n");
        printf("more info \n");

        // if(data extraction started == false)
            // data extraction start => 
                // - set all the parameters
                // - enable all the interrupts
        // mostra a schermo l'avanzamento del data extractor
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

        // if(!USB.transfer)
            // USB.transfer = true
            // call USB function/transfer 
            // while(1){}
 
        // mostra a schermo enable USB transfer while you are in this menù
        // da aggiungere che quando si esce da questo selezione l'usb tasnfer si blocca
    }
}

///////////////////////////////   settings state    ////////////////////////////////////////////

void fn_TEMPERATURA(){
    settings.final_depth = true;
    printf("TEMPERATURA \n");
    printf("more info:\n");
    printf("sampling rate:  %d \n",tmp_settings.Temp_Sampling_Rate);
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
        printf("ACC: X, Y, Z e sampling rate \n");
        printf("selettore su:");
        switch (settings.current_state_accelerometer)
        {
        case STATE_X_ACC:
            printf("\t STATE_X_ACC \n");
            break;
        case STATE_Y_ACC:
            printf("\t STATE_Y_ACC \n");
            break;
        case STATE_Z_ACC:
            printf("\t STATE_Z_ACC \n");
            break;
        case STATE_SAMPLING_RATE_ACC:
            printf("\t case STATE_SAMPLING_RATE_ACC \n");
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
        case STATE_SAMPLING_RATE_GPS:
            printf("\t case STATE_SAMPLING_RATE_GPS \n");
            break;
        }
        // mostrare a schermo il menù con i vari attributi
    }
}


///////////////////////////////   state GPS - attribute    ////////////////////////////////////////////

void fn_GPS_SAMPLING_RATE(){
    settings.final_depth = true;
    printf("GPS \n");
    printf("more info:\n");
    printf("sampling rate:  %d \n",tmp_settings.GPS_Sampling_Rate);
    // mostra a schermo il valore temporaneo di SAMPLING_RATE
}

///////////////////////////////   state ACC - attribute    ////////////////////////////////////////////
void fn_ACC_X(){
    settings.final_depth = true;
    printf("ACCELEROMETER \n");
    printf("more info:\n");
    printf("X(=0 if not enabled, =1 if enabled):  %d  \n",tmp_settings.Acc_X);
    // mostra a schermo il valore temporaneo di X (ENABLE OR DISABLE)
}
void fn_ACC_Y(){
    settings.final_depth = true;
    printf("ACCELEROMETER \n");
    printf("more info:\n");
    printf("Y(=0 if not enabled, =1 if enabled):  %d \n",tmp_settings.Acc_Y);
    // mostra a schermo il valore temporaneo di y (ENABLE OR DISABLE)
}
void fn_ACC_Z(){
    settings.final_depth = true;
    printf("ACCELEROMETER \n");
    printf("more info:\n");
    printf("Z(=0 if not enabled, =1 if enabled):  %d \n",tmp_settings.Acc_Z);
    // mostra a schermo il valore temporaneo di z (ENABLE OR DISABLE)
}
void fn_ACC_SAMPLING_RATE(){
    settings.final_depth = true;
    printf("ACCELEROMETER \n");
    printf("more info:\n");
    printf("sampling rate:  %d  \n",tmp_settings.Acc_Sampling_Rate);
    // mostra a schermo il valore temporaneo di SAMPLING_RATE
}





/**************************************** buttons *************************
 * 
 * 
 * 
*/
void left_button(){
    if(settings.final_depth){
        // call buzzer(discard settings)
        tmp_settings = settings;
        settings.final_depth = false;
    }

    if(!settings.start_depth){
        // call buzzer(normal sound)
        settings.depth--;
        if(settings.depth == 0){
            settings.start_depth = true;
        }
    }
    else{
        // call buzzer(error)
    }

}

void rigth_button(){
    // printf("%d", settings.final_depth);
    settings.start_depth = false;
    if(settings.final_depth){
        // call buzzer(save settings)
        settings.depth--;
        settings = tmp_settings;
        settings.final_depth = false;
    }
    else{
        // call buzzer(normal sound)
        settings.depth++;
    }
}

void up_button(){
    if(!settings.final_depth){
        switch (settings.depth){
            case 0:
                if(!(settings.current_state_function == NUM_STATES_FUNCTION-1)){
                    settings.current_state_function ++;
                    // call buzzer(normal sound)
                }
                else {
                    // call buzzer(error)
                }
                break;
            case 1:
                if(!(settings.current_state_settings == NUM_STATES_SETTINGS-1)){
                    settings.current_state_settings ++;
                    // call buzzer(normal sound)
                }
                else {
                    // call buzzer(error)
                }
                break;
            case 2:
                switch(settings.current_state_settings){
                    case STATE_ACCELEROMETRO:
                        if(!(settings.current_state_accelerometer == NUM_STATES_ACCELEROMETER-1)){
                            settings.current_state_accelerometer ++;
                            // call buzzer(normal sound)
                        }
                        else {
                            // call buzzer(error)
                        }
                        break;
                    case STATE_GPS:
                        if(!(settings.current_state_GPS == NUM_STATES_ACCELEROMETER-1)){
                            settings.current_state_GPS ++;
                            // call buzzer(normal sound)
                        }
                        else {
                            // call buzzer(error)
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
                        if(tmp_settings.Temp_Sampling_Rate < MAX_SAMPLING_RATE){
                            tmp_settings.Temp_Sampling_Rate++;
                            // call buzzer(normal sound)
                        }
                        else {
                            // call buzzer(error)
                        }                
                    break;
                    case STATE_ACCELEROMETRO:
                        switch (settings.current_state_accelerometer){
                            case STATE_X_ACC:
                                if(tmp_settings.Acc_X){
                                    tmp_settings.Acc_X = false;
                                    // call buzzer(normal sound)
                                }
                                else{
                                    tmp_settings.Acc_X = true;
                                    // call buzzer(normal sound)
                                }
                            break;
                            case STATE_Y_ACC:
                                if(tmp_settings.Acc_Y){
                                    tmp_settings.Acc_Y = false;
                                    // call buzzer(normal sound)
                                }
                                else{
                                    tmp_settings.Acc_Y = true;
                                    // call buzzer(normal sound)
                                }
                            break;
                            case STATE_Z_ACC:
                                if(tmp_settings.Acc_Z){
                                    tmp_settings.Acc_Z = false;
                                    // call buzzer(normal sound)
                                }
                                else{
                                    tmp_settings.Acc_Z = true;
                                    // call buzzer(normal sound)
                                }
                            break;
                            case STATE_SAMPLING_RATE_ACC:
                                if(tmp_settings.Acc_Sampling_Rate < MAX_SAMPLING_RATE){
                                    tmp_settings.Acc_Sampling_Rate++;
                                    // call buzzer(normal sound)
                                }
                                else {
                                    // call buzzer(error)
                                }
                            break;                            
                        }
                    break;
                    case STATE_GPS:
                        switch (settings.current_state_GPS){
                            case STATE_SAMPLING_RATE_ACC:
                                if(tmp_settings.GPS_Sampling_Rate < MAX_SAMPLING_RATE){
                                    tmp_settings.GPS_Sampling_Rate++;
                                    // call buzzer(normal sound)
                                }
                                else {
                                    // call buzzer(error)
                                }
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
                    // call buzzer(normal sound)
                }
                else {
                    // call buzzer(error)
                }
                break;
            case 1:
                if(!(settings.current_state_settings == 0)){
                    settings.current_state_settings --;
                    // call buzzer(normal sound)
                }
                else {
                    // call buzzer(error)
                }
                break;
            case 2:
                switch (settings.current_state_settings){
                case STATE_ACCELEROMETRO:
                    if(!(settings.current_state_accelerometer == 0)){
                        settings.current_state_accelerometer --;
                        // call buzzer(normal sound)
                    }
                    else {
                        // call buzzer(error)
                    }
                    break;
                case STATE_GPS:
                    if(!(settings.current_state_GPS == 0)){
                        settings.current_state_GPS --;
                        // call buzzer(normal sound)
                    }
                    else {
                        // call buzzer(error)
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
                        if(tmp_settings.Temp_Sampling_Rate > 0){
                            tmp_settings.Temp_Sampling_Rate--;
                                // call buzzer(normal sound)
                        }
                        else {
                            // call buzzer(error)
                        }
                        break;
                    case STATE_ACCELEROMETRO:
                        switch (settings.current_state_accelerometer){
                            case STATE_X_ACC:
                                if(tmp_settings.Acc_X){
                                    tmp_settings.Acc_X = false;
                                    // call buzzer(normal sound)
                                }
                                else{
                                    tmp_settings.Acc_X = true;
                                    // call buzzer(normal sound)
                                }
                            break;
                            case STATE_Y_ACC:
                                if(tmp_settings.Acc_Y){
                                    tmp_settings.Acc_Y = false;
                                    // call buzzer(normal sound)
                                }
                                else{
                                    tmp_settings.Acc_Y = true;
                                    // call buzzer(normal sound)
                                }
                            break;
                            case STATE_Z_ACC:
                                if(tmp_settings.Acc_Z){
                                    tmp_settings.Acc_Z = false;
                                    // call buzzer(normal sound)
                                }
                                else{
                                    tmp_settings.Acc_Z = true;
                                    // call buzzer(normal sound)
                                }
                            break;
                            case STATE_SAMPLING_RATE_ACC:
                                if(tmp_settings.Acc_Sampling_Rate > 0){
                                    tmp_settings.Acc_Sampling_Rate--;
                                    // call buzzer(normal sound)
                                }
                                else {
                                    // call buzzer(error)
                                }
                            break;                            
                        }
                    break;
                    case STATE_GPS:
                        switch (settings.current_state_GPS){
                            case STATE_SAMPLING_RATE_ACC:
                                if(tmp_settings.GPS_Sampling_Rate > 0){
                                    tmp_settings.GPS_Sampling_Rate--;
                                // call buzzer(normal sound)
                            }
                            else {
                                // call buzzer(error)
                            }
                        }                 
                    break;
                }
            break;
        }
    }    
}


/*************************** sleep mode **************************/
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

/****************** interrput handler *************/
bool interrompi = false;

#define time_interval_between_button_push 1000000 
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
            interrompi = true;
            time_bt_up = time;
            up_button();
            // printf("clicked up \n");
        }
        break;
    case 23:
        difference = time - time_bt_down;
        if(difference > time_interval_between_button_push){
            interrompi = true;
            time_bt_down = time;
            down_button();
            // printf("clicked down \n");
        }
        break;
    case 24:
        difference = time - time_bt_right;
        if(difference > time_interval_between_button_push){
            interrompi = true;
            time_bt_right = time;
            rigth_button();
            // printf("clicked right \n");
        }
        break;
    case 25:
        difference = time - time_bt_left;
        if(difference > time_interval_between_button_push){
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
            // printf("%d \n",settings.depth);
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
