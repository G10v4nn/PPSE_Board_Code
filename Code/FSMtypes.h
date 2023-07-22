#ifndef FSMTYPES_INCLUDED
#define FSMTYPES_INCLUDED

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
    STATE_SAMPLING_RATE,
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
    NUM_STATES_ACCELEROMETER
}State_Accelerometer;

typedef struct{
    State_Accelerometer state;
    void (*state_function)(void);
} StateMachine_Accelerometer_t;

typedef enum {
    // add features
    GRADI_1_GPS,
    GRADI_2_GPS,
    ALTITUDINE_GPS,
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
    uint32_t Sampling_Rate;

    // accelerometer
    bool Acc_X;
    bool Acc_Y;
    bool Acc_Z;

    // temperatura
    bool Temp;

    // gps
    bool Gradi_1_GPS;
    bool Gradi_2_GPS;
    bool Altitudine_GPS;

    /* data */
} Settings_storage;


/*******************************************************************/
/* definizioni delle variabili globali */

Settings_storage settings = {
    
    STATE_INIT,             // State_Menu current_state_menu;
    STATE_SETTINGS,         // State_Function current_state_function;
    STATE_TEMPERATURA,      // State_Settings current_state_settings;
    STATE_X_ACC,            // State_Accelerometer current_state_accelerometer;
    ALTITUDINE_GPS,         // State_GPS current_state_GPS;
    
    true,                   // bool start_depth;
    0,                      // int depth;
    false,                  // bool final_depth;
    1,                   //     uint32_t Sampling_Rate;

    // accelerometer
    true,                   // bool Acc_X;
    true,                   // bool Acc_Y;
    true,                   // bool Acc_Z;

    // temperatura
    true,                   // int Temp_Sampling_Rate;

    // gps
    true,                   // bool Gradi_1_GPS;
    true,                   // bool Gradi_2_GPS;
    true,                   // bool Altitudine_GPS;

};

Settings_storage tmp_settings = { };

#endif