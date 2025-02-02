#ifndef CAPABILITIES_H_
#define CAPABILITIES_H_
#include "serial.h"


//Hardware capabilities for mocked logger

#define TICK_RATE_HZ			1000
#define MS_PER_TICK 5
//configuration
#define MAX_TRACKS				240
#define MAX_SECTORS				20
#define SCRIPT_MEMORY_LENGTH	10240
#define MAX_VIRTUAL_CHANNELS	10

//Input / output Channels
#define ANALOG_CHANNELS 		8
#define IMU_CHANNELS			7
#define	GPIO_CHANNELS			3
#define TIMER_CHANNELS			3
#define PWM_CHANNELS			4
#define CAN_CHANNELS			2
#define CONNECTIVITY_CHANNELS	2

//sample rates
#define MAX_SENSOR_SAMPLE_RATE	1000
#define MAX_GPS_SAMPLE_RATE		50
#define MAX_OBD2_SAMPLE_RATE	1000

//logging
#define LOG_BUFFER_SIZE			1024

//system info
#define DEVICE_NAME    "RCP_SIM"
#define FRIENDLY_DEVICE_NAME "RaceCapture/Pro Sim"
#define COMMAND_PROMPT "RaceCapture/Pro Sim"
#define VERSION_STR MAJOR_REV_STR "." MINOR_REV_STR "." BUGFIX_REV_STR
#define WELCOME_MSG "Welcome to RaceCapture/Pro Sim: Firmware Version " VERSION_STR



#endif /* CAPABILITIES_H_ */
