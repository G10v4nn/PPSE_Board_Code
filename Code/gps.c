#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"

#include "datatypes.h"
#include "gps_types.h"
#include "gps.h"

#define GPS_EN 18
#define GPS_RX 4
#define GPS_TX 5

#define LANDING_ALTITUDE    100

struct TGPS GPS;

char Hex(char Character)
{
	char HexTable[] = "0123456789ABCDEF";
	
	return HexTable[Character];
}

int BuildSentence(struct TGPS *GPS, char *TxLine, const char *PayloadID)
{
	static unsigned int SentenceCounter=0;
    int Count, i, j;
    unsigned char c;
    unsigned int CRC, xPolynomial;
    char CRCString[8];
	
    SentenceCounter++;
	

    sprintf(TxLine,
            // SENTENCE_LENGTH-6,
            "$$%s,%d,%02d:%02d:%02d,%.5f,%.5f,%05.5ld,%u,%.1f,%.1f,%.1f,%.0f,%.1f,%.2f,%7.5f,%7.5f,%3.1f,%d",
            PayloadID,
            SentenceCounter,
			GPS->Hours, GPS->Minutes, GPS->Seconds,
            GPS->Latitude,
            GPS->Longitude,
            GPS->Altitude,
			GPS->Satellites,
            GPS->BatteryVoltage,
			GPS->InternalTemperature,
			GPS->ExternalTemperature,
			GPS->Pressure,
			GPS->Humidity,
			GPS->CDA,
			GPS->PredictedLatitude,
			GPS->PredictedLongitude,
			GPS->PredictedLandingSpeed,
			GPS->TimeTillLanding
            );
            
    Count = strlen(TxLine);

    CRC = 0xffff;           // Seed
    xPolynomial = 0x1021;
   
     for (i = 2; i < Count; i++)
     {   // For speed, repeat calculation instead of looping for each bit
        CRC ^= (((unsigned int)TxLine[i]) << 8);
        for (j=0; j<8; j++)
        {
            if (CRC & 0x8000)
                CRC = (CRC << 1) ^ 0x1021;
            else
                CRC <<= 1;
        }
     }

    TxLine[Count++] = '*';
    TxLine[Count++] = Hex((CRC >> 12) & 15);
    TxLine[Count++] = Hex((CRC >> 8) & 15);
    TxLine[Count++] = Hex((CRC >> 4) & 15);
    TxLine[Count++] = Hex(CRC & 15);
  	TxLine[Count++] = '\n';  
	TxLine[Count++] = '\0';
	
	return strlen(TxLine) + 1;
}

int GPSChecksumOK(unsigned char *Buffer, int Count)
{
  unsigned char XOR, i, c;

  XOR = 0;
  for (i = 1; i < (Count-4); i++)
  {
    c = Buffer[i];
    XOR ^= c;
  }

  return (Buffer[Count-4] == '*') && (Buffer[Count-3] == Hex(XOR >> 4)) && (Buffer[Count-2] == Hex(XOR & 15));
}

void SendUBX(unsigned char *msg, int len)
{
	int i;
	
	for (i=0; i<len; i++)
	{
		uart_putc(uart1, msg[i]);
	}
}

float FixPosition(float Position)
{
	float Minutes, Seconds;
	
	Position = Position / 100;
	
	Minutes = trunc(Position);
	Seconds = fmod(Position, 1);

	return Minutes + Seconds * 5 / 3;
}

void ProcessLine(struct TGPS *GPS, char *Buffer, int Count)
{
    float utc_time, latitude, longitude, hdop, altitude, speed, course;
	int lock, satellites, date;
	char active, ns, ew, units, speedstring[16], coursestring[16];
	
    if (GPSChecksumOK(Buffer, Count))
	{
		satellites = 0;
	
		if (strncmp(Buffer+3, "GGA", 3) == 0)
		{
			if (sscanf(Buffer+7, "%f,%f,%c,%f,%c,%d,%d,%f,%f,%c", &utc_time, &latitude, &ns, &longitude, &ew, &lock, &satellites, &hdop, &altitude, &units) >= 1)
			{	
				// $GPGGA,124943.00,5157.01557,N,00232.66381,W,1,09,1.01,149.3,M,48.6,M,,*42
				GPS->Time = utc_time;
				GPS->Hours = GPS->Time / 10000;
				GPS->Minutes = (GPS->Time / 100) % 100;
				GPS->Seconds = GPS->Time % 100;
				GPS->SecondsInDay = GPS->Hours * 3600 + GPS->Minutes * 60 + GPS->Seconds;					

				if (GPS->UseHostPosition)
				{
					GPS->UseHostPosition--;
				}
				else if (satellites >= 4)
				{
					GPS->Latitude = FixPosition(latitude);
					if (ns == 'S') GPS->Latitude = -GPS->Latitude;
					GPS->Longitude = FixPosition(longitude);
					if (ew == 'W') GPS->Longitude = -GPS->Longitude;
					GPS->Altitude = altitude;
				}

				GPS->Satellites = satellites;     
			}
		}
		else if (strncmp(Buffer+3, "RMC", 3) == 0)
		{
			speedstring[0] = '\0';
			coursestring[0] = '\0';
			if (sscanf(Buffer+7, "%f,%c,%f,%c,%f,%c,%[^','],%[^','],%d", &utc_time, &active, &latitude, &ns, &longitude, &ew, speedstring, coursestring, &date) >= 7)
			{
				// $GPRMC,124943.00,A,5157.01557,N,00232.66381,W,0.039,,200314,,,A*6C

				speed = atof(speedstring);
				course = atof(coursestring);
				
				GPS->Speed = (int)speed;
				GPS->Direction = (int)course;
			}
		}
		else if (strncmp(Buffer+3, "GSV", 3) == 0)
        {
            // Disable GSV
            printf("Disabling GSV\r\n");
            unsigned char setGSV[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x39 };
            SendUBX(setGSV, sizeof(setGSV));
        }
		else if (strncmp(Buffer+3, "GLL", 3) == 0)
        {
            // Disable GLL
            printf("Disabling GLL\r\n");
            unsigned char setGLL[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x2B };
            SendUBX(setGLL, sizeof(setGLL));
        }
		else if (strncmp(Buffer+3, "GSA", 3) == 0)
        {
            // Disable GSA
            printf("Disabling GSA\r\n");
            unsigned char setGSA[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x32 };
            SendUBX(setGSA, sizeof(setGSA));
        }
		else if (strncmp(Buffer+3, "VTG", 3) == 0)
        {
            // Disable VTG
            printf("Disabling VTG\r\n");
            unsigned char setVTG[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x47};
            SendUBX(setVTG, sizeof(setVTG));
        }
        else
        {
            printf("Unknown NMEA sentence: %s\n", Buffer);
        }
    }
    else
    {
       printf("Bad checksum\r\n");
	}
}

void check_gps(struct TGPS *GPS)
{
	static unsigned char Line[100];
	static int Length=0;
	while (uart_is_readable(uart1))
	{
		char Character;
		Character = uart_getc(uart1);
		if (Character == '$')
		{
			Line[0] = Character;
			Length = 1;
		}
		else if (Length > 90)
		{
			Length = 0;
		}
		else if ((Length > 0) && (Character != '\r'))
		{
			Line[Length++] = Character;
			if (Character == '\n')
			{
				Line[Length] = '\0';
				printf("%s", Line);
				ProcessLine(GPS, Line, Length);
				Length = 0;
			}
		}
	}
}

void init_gps(void)
{
    //turn on GPS
	gpio_init(GPS_EN);
	gpio_set_dir(GPS_EN, GPIO_OUT);
	gpio_put(GPS_EN, 0);

	// Initialise UART 1
	uart_init(uart1, 9600);
	gpio_set_function(GPS_RX, GPIO_FUNC_UART);
	gpio_set_function(GPS_TX , GPIO_FUNC_UART);
}

void get_gps(struct Data_storage *Data){
    check_gps(&GPS);
	Data->time = GPS.Time;
    Data->Hours = GPS.Hours;
    Data->Minutes = GPS.Minutes;
    Data->Seconds = GPS.Seconds;

    Data->Longitude = GPS.Longitude;
    Data->Latitude = GPS.Latitude;

    Data->Altitude = GPS.Altitude;
}