#ifndef GPSTYPES_INCLUDED
#define GPSTYPES_INCLUDED

struct TGPS
{
	long Time;							// Time as read from GPS, as an integer but 12:13:14 is 121314
	long SecondsInDay;					// Time in seconds since midnight.  Used for APRS timing, and LoRa timing in TDM mode
	int Hours, Minutes, Seconds;
	float Longitude, Latitude;
	long Altitude, MinimumAltitude, MaximumAltitude, PreviousAltitude;
	unsigned int Satellites;
	int Speed;
	int Direction;
	float AscentRate;
	float BatteryVoltage;
	float InternalTemperature;
	float ExternalTemperature;
	float Pressure;
	float Humidity;
	float PredictedLongitude, PredictedLatitude;
	float CDA;
	int UseHostPosition;
	int TimeTillLanding;
	float PredictedLandingSpeed;
};

#endif