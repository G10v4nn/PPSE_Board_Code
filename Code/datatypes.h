#ifndef TYPES_INCLUDED
#define TYPES_INCLUDED

/*******************************************************************/
/* Define struct as data storage of all sensors */

struct Data_storage{
    // gps
    float time;    
    int Hours, Minutes, Seconds;
	float Longitude, Latitude;
    long Altitude;

    // accelerometer
    float x_acceleration;
    float y_acceleration;
    float z_acceleration;

    // temperature
    float temperature;

    /* data */
};

#endif