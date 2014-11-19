#include "loggerConfig.h"
#include "modp_numtoa.h"
#include "mod_string.h"
#include "memory.h"
#include "printk.h"
#include "virtual_channel.h"

#include <stdbool.h>

#ifndef RCP_TESTING
#include "memory.h"
static const volatile LoggerConfig g_savedLoggerConfig  __attribute__((section(".config\n\t#")));
#else
static LoggerConfig g_savedLoggerConfig;
#endif

static LoggerConfig g_workingLoggerConfig;

static void resetVersionInfo(VersionInfo *vi) {
   vi->major = MAJOR_REV;
   vi->minor = MINOR_REV;
   vi->bugfix = BUGFIX_REV;
}

static void resetPwmClkFrequency(unsigned short *pwmClkFreq) {
   *pwmClkFreq = DEFAULT_PWM_CLOCK_FREQUENCY;
}

/**
 * Prints a string prefix and add an int suffix to dest buffer.
 */
static void sPrintStrInt(char *dest, const char *str, const unsigned int i) {
   char iStr[3];
   const int idx = strlen(str);

   modp_itoa10(i, iStr);
   strcpy(dest, str);
   strcpy(dest + idx, iStr);
}

static void resetTimeConfig(struct TimeConfig *tc) {
   tc[0] = (struct TimeConfig) DEFAULT_UPTIME_TIME_CONFIG;
   tc[1] = (struct TimeConfig) DEFAULT_UTC_MILLIS_TIME_CONFIG;
}

static void resetAdcConfig(ADCConfig cfg[]) {
   // All but the last one are zeroed out.
   for (size_t i = 0; i < CONFIG_ADC_CHANNELS; ++i) {
      ADCConfig *c = cfg + i;
      *c = (ADCConfig) DEFAULT_ADC_CONFIG;
      sPrintStrInt(c->cfg.label, "Analog", i + 1);
      strcpy(c->cfg.units, "Volts");
   }

   // Now update the battery config
   cfg[7] = (ADCConfig) BATTERY_ADC_CONFIG;
}

static void resetPwmConfig(PWMConfig cfg[]) {
   for (size_t i = 0; i < CONFIG_PWM_CHANNELS; ++i) {
      PWMConfig *c = cfg + i;
      *c = (PWMConfig) DEFAULT_PWM_CONFIG;
      sPrintStrInt(c->cfg.label, "PWM", i + 1);
   }
}

static void resetGpioConfig(GPIOConfig cfg[]) {
   for (size_t i = 0; i < CONFIG_GPIO_CHANNELS; ++i) {
      GPIOConfig *c = cfg + i;
      *c = (GPIOConfig) DEFAULT_GPIO_CONFIG;
      sPrintStrInt(c->cfg.label, "GPIO", i + 1);
   }
}

static void resetTimerConfig(TimerConfig cfg[]) {
   for (size_t i = 0; i < CONFIG_TIMER_CHANNELS; ++i) {
      TimerConfig *c = cfg + i;
      *c = (TimerConfig) DEFAULT_FREQUENCY_CONFIG;
      sPrintStrInt(c->cfg.label, "RPM", i + 1);
   }

   // Make Channel 1 the default RPM config.
   cfg[0].cfg = (ChannelConfig) DEFAULT_RPM_CHANNEL_CONFIG;
}

static void resetImuConfig(ImuConfig cfg[]) {
   const char *names[] = {"AccelX", "AccelY", "AccelZ"};

   for (size_t i = 0; i < 3; ++i) {
      ImuConfig *c = cfg + i;
      *c = (ImuConfig) DEFAULT_IMU_CONFIG;
      strcpy(c->cfg.label, names[i]);

      // Channels go X, Y, Z.  Works perfectly with our counter.
      c->physicalChannel = i;
   }

   cfg[3] = (ImuConfig) DEFAULT_GYRO_YAW_AXIS_CONFIG;
}

static void resetCanConfig(CANConfig *cfg) {
   *cfg = (CANConfig) DEFAULT_CAN_CONFIG;
}

static void resetOBD2Config(OBD2Config *cfg) {
   memset(cfg, 0, sizeof(OBD2Config));
   cfg->obd2SampleRate = SAMPLE_10Hz;

   for (int i = 0; i < OBD2_CHANNELS; ++i) {
      PidConfig *c = &cfg->pids[i];
      memset(c, 0, sizeof(PidConfig));
      sPrintStrInt(c->cfg.label, "OBD2 Pid ", i + 1);
   }
}

static void resetGPSConfig(GPSConfig *cfg) {
   *cfg = (GPSConfig) DEFAULT_GPS_CONFIG;
}

static void resetLapConfig(LapConfig *cfg) {
   *cfg = (LapConfig) DEFAULT_LAP_CONFIG;
}

static void resetTrackConfig(TrackConfig *cfg) {
   memset(cfg, 0, sizeof(TrackConfig));
   cfg->radius = DEFAULT_TRACK_TARGET_RADIUS;
   cfg->auto_detect = DEFAULT_TRACK_AUTO_DETECT;
}

static void resetBluetoothConfig(BluetoothConfig *cfg) {
   *cfg = (BluetoothConfig) DEFAULT_BT_CONFIG;
}

static void resetCellularConfig(CellularConfig *cfg) {
   memset(cfg, 0, sizeof(CellularConfig));
   cfg->cellEnabled = CELL_ENABLED;
   strcpy(cfg->apnHost, DEFAULT_APN_HOST);
}

static void resetTelemetryConfig(TelemetryConfig *cfg) {
   memset(cfg, 0, sizeof(TelemetryConfig));
   cfg->backgroundStreaming = BACKGROUND_STREAMING_ENABLED;
   strcpy(cfg->telemetryServerHost, DEFAULT_TELEMETRY_SERVER_HOST);
}

static void resetConnectivityConfig(ConnectivityConfig *cfg) {
   resetBluetoothConfig(&cfg->bluetoothConfig);
   resetCellularConfig(&cfg->cellularConfig);
   resetTelemetryConfig(&cfg->telemetryConfig);
}

bool isHigherSampleRate(const int contender, const int champ) {
   // Contender can't win here.  Ever.
   if (contender == SAMPLE_DISABLED)
      return false;

   // Champ defaults in this case.  Contender need only show up.
   if (champ == SAMPLE_DISABLED)
      return contender != SAMPLE_DISABLED;

   return contender < champ;
}

int getHigherSampleRate(const int a, const int b) {
   return isHigherSampleRate(a, b) ? a : b;
}

int flash_default_logger_config(void){
	pr_info("flashing default logger config...");

   LoggerConfig *lc = &g_workingLoggerConfig;

   resetVersionInfo(&lc->RcpVersionInfo);
   resetPwmClkFrequency(&lc->PWMClockFrequency);
   resetTimeConfig(lc->TimeConfigs);
   resetAdcConfig(lc->ADCConfigs);
   resetPwmConfig(lc->PWMConfigs);
   resetGpioConfig(lc->GPIOConfigs);
   resetTimerConfig(lc->TimerConfigs);
   resetImuConfig(lc->ImuConfigs);
   resetCanConfig(&lc->CanConfig);
   resetOBD2Config(&lc->OBD2Configs);
   resetGPSConfig(&lc->GPSConfigs);
   resetLapConfig(&lc->LapConfigs);
   resetTrackConfig(&lc->TrackConfigs);
   resetConnectivityConfig(&lc->ConnectivityConfigs);
   strcpy(lc->padding_data, "");

	int result = flashLoggerConfig();

	pr_info(result == 0 ? "success\r\n" : "failed\r\n");

	return result;
}

int flashLoggerConfig(void){
	return memory_flash_region((void *) &g_savedLoggerConfig,
                              (void *) &g_workingLoggerConfig,
                              sizeof (LoggerConfig));
}

static bool checkFlashDefaultConfig(void){
	size_t major_version_changed = g_savedLoggerConfig.RcpVersionInfo.major != MAJOR_REV;
	size_t minor_version_changed = g_savedLoggerConfig.RcpVersionInfo.minor != MINOR_REV;

	if (!major_version_changed && !minor_version_changed)
      return false;

   pr_info("Major or minor firmware version changed\r\n");
   flash_default_logger_config();
   return true;
}

static void loadWorkingLoggerConfig(void){
	memcpy((void *) &g_workingLoggerConfig,
          (void *) &g_savedLoggerConfig, sizeof(LoggerConfig));
}

void initialize_logger_config(){
	checkFlashDefaultConfig();
	loadWorkingLoggerConfig();
}

const LoggerConfig * getSavedLoggerConfig(){
	return (LoggerConfig *) &g_savedLoggerConfig;
}

LoggerConfig * getWorkingLoggerConfig(){
	return &g_workingLoggerConfig;
}

void calculateTimerScaling(unsigned int clockHz, TimerConfig *timerConfig){
	unsigned int clock = clockHz / timerConfig->timerDivider;
	clock = clock / timerConfig->pulsePerRevolution;
	timerConfig->calculatedScaling = clock;
}

int getConnectivitySampleRateLimit(){
	ConnectivityConfig *connConfig = &getWorkingLoggerConfig()->ConnectivityConfigs;
	int sampleRateLimit = connConfig->cellularConfig.cellEnabled ? SLOW_LINK_MAX_TELEMETRY_SAMPLE_RATE : FAST_LINK_MAX_TELEMETRY_SAMPLE_RATE;
	return sampleRateLimit;
}

// STIEG: FIX ME! This can be done with math
int encodeSampleRate(int sampleRate){

	switch(sampleRate){
		case 200:
			return SAMPLE_200Hz;
		case 100:
			return SAMPLE_100Hz;
		case 50:
			return SAMPLE_50Hz;
		case 25:
			return SAMPLE_25Hz;
		case 10:
			return SAMPLE_10Hz;
		case 5:
			return SAMPLE_5Hz;
		case 1:
			return SAMPLE_1Hz;
		default:
		case 0:
			return SAMPLE_DISABLED;
	}
}

// STIEG: FIX ME!  This can be done with math.
int decodeSampleRate(int sampleRateCode){

	switch(sampleRateCode){
		case SAMPLE_200Hz:
			return 200;
		case SAMPLE_100Hz:
			return 100;
		case SAMPLE_50Hz:
			return 50;
		case SAMPLE_25Hz:
			return 25;
		case SAMPLE_10Hz:
			return 10;
		case SAMPLE_5Hz:
			return 5;
		case SAMPLE_1Hz:
			return 1;
		default:
		case SAMPLE_DISABLED:
			return 0;
	}
}

unsigned char filterAnalogScalingMode(unsigned char mode){
	switch(mode){
		case SCALING_MODE_LINEAR:
			return SCALING_MODE_LINEAR;
		case SCALING_MODE_MAP:
			return SCALING_MODE_MAP;
		default:
		case SCALING_MODE_RAW:
			return SCALING_MODE_RAW;
	}
}

unsigned char filterSdLoggingMode(unsigned char mode){
	switch (mode){
		case SD_LOGGING_MODE_CSV:
			return SD_LOGGING_MODE_CSV;
		default:
		case SD_LOGGING_MODE_DISABLED:
			return SD_LOGGING_MODE_DISABLED;
	}
}

char filterGpioMode(int value){
	switch(value){
		case CONFIG_GPIO_OUT:
			return CONFIG_GPIO_OUT;
		case CONFIG_GPIO_IN:
		default:
			return CONFIG_GPIO_IN;
	}
}

char filterPwmOutputMode(int value){
	switch(value){
		case MODE_PWM_ANALOG:
			return MODE_PWM_ANALOG;
		case MODE_PWM_FREQUENCY:
		default:
			return MODE_PWM_FREQUENCY;
	}
}

char filterPwmLoggingMode(int config){
	switch (config){
		case MODE_LOGGING_PWM_PERIOD:
			return MODE_LOGGING_PWM_PERIOD;
		case MODE_LOGGING_PWM_DUTY:
			return MODE_LOGGING_PWM_DUTY;
		case MODE_LOGGING_PWM_VOLTS:
		default:
			return MODE_LOGGING_PWM_VOLTS;
	}
}

unsigned char filterPulsePerRevolution(unsigned char pulsePerRev){
	return pulsePerRev == 0 ? 1 : pulsePerRev;
}

unsigned short filterTimerDivider(unsigned short divider){
	switch(divider){
	case TIMER_MCK_2:
		return TIMER_MCK_2;
	case TIMER_MCK_8:
		return TIMER_MCK_8;
	case TIMER_MCK_32:
		return TIMER_MCK_32;
	case TIMER_MCK_128:
		return TIMER_MCK_128;
	case TIMER_MCK_1024:
		return TIMER_MCK_1024;
	default:
		return TIMER_MCK_128;
	}
}
char filterTimerMode(int mode){
	switch (mode){
		case MODE_LOGGING_TIMER_RPM:
			return MODE_LOGGING_TIMER_RPM;
		case MODE_LOGGING_TIMER_PERIOD_MS:
			return MODE_LOGGING_TIMER_PERIOD_MS;
		case MODE_LOGGING_TIMER_PERIOD_USEC:
			return MODE_LOGGING_TIMER_PERIOD_USEC;
		default:
		case MODE_LOGGING_TIMER_FREQUENCY:
			return MODE_LOGGING_TIMER_FREQUENCY;
	}
}

int filterImuChannel(int config){
	switch(config){
		case IMU_CHANNEL_Y:
			return IMU_CHANNEL_Y;
		case IMU_CHANNEL_Z:
			return IMU_CHANNEL_Z;
		case IMU_CHANNEL_YAW:
			return IMU_CHANNEL_YAW;
		default:
		case IMU_CHANNEL_X:
			return IMU_CHANNEL_X;
	}
}

int filterImuRawValue(int imuRawValue){
	if (imuRawValue > MAX_IMU_RAW){
		imuRawValue = MAX_IMU_RAW;
	} else if (imuRawValue < MIN_IMU_RAW){
		imuRawValue = MIN_IMU_RAW;
	}
	return imuRawValue;
}

int filterImuMode(int mode){
	switch (mode){
		case MODE_IMU_DISABLED:
			return MODE_IMU_DISABLED;
		case MODE_IMU_INVERTED:
			return MODE_IMU_INVERTED;
		default:
		case MODE_IMU_NORMAL:
			return MODE_IMU_NORMAL;
	}
}

unsigned short filterPwmDutyCycle(int dutyCycle){
	if (dutyCycle > MAX_PWM_DUTY_CYCLE){
		dutyCycle = MAX_PWM_DUTY_CYCLE;
	} else if (dutyCycle < MIN_PWM_DUTY_CYCLE){
		dutyCycle = MIN_PWM_DUTY_CYCLE;
	}
	return dutyCycle;
}

unsigned short filterPwmPeriod(int period){
	if (period > MAX_PWM_PERIOD){
		period = MAX_PWM_PERIOD;
	} else if (period < MIN_PWM_PERIOD){
		period = MIN_PWM_PERIOD;
	}
	return period;
}

int filterPwmClockFrequency(int freq){
	if (freq > MAX_PWM_CLOCK_FREQUENCY){
		freq = MAX_PWM_CLOCK_FREQUENCY;
	} else if (freq < MIN_PWM_CLOCK_FREQUENCY){
		freq = MIN_PWM_CLOCK_FREQUENCY;
	}
	return freq;
}

PWMConfig * getPwmConfigChannel(int channel){
	PWMConfig * c = NULL;
	if (channel >= 0 && channel < CONFIG_PWM_CHANNELS){
		c = &(getWorkingLoggerConfig()->PWMConfigs[channel]);
	}
	return c;
}

TimerConfig * getTimerConfigChannel(int channel){
	TimerConfig * c = NULL;
	if (channel >=0 && channel < CONFIG_TIMER_CHANNELS){
		c = &(getWorkingLoggerConfig()->TimerConfigs[channel]);
	}
	return c;
}

ADCConfig * getADCConfigChannel(int channel){
	ADCConfig *c = NULL;
	if (channel >=0 && channel < CONFIG_ADC_CHANNELS){
		c = &(getWorkingLoggerConfig()->ADCConfigs[channel]);
	}
	return c;
}

GPIOConfig * getGPIOConfigChannel(int channel){
	GPIOConfig *c = NULL;
	if (channel >=0 && channel < CONFIG_GPIO_CHANNELS){
		c = &(getWorkingLoggerConfig()->GPIOConfigs[channel]);
	}
	return c;
}

ImuConfig * getImuConfigChannel(int channel){
	ImuConfig * c = NULL;
	if (channel >= 0 && channel < CONFIG_IMU_CHANNELS){
		c = &(getWorkingLoggerConfig()->ImuConfigs[channel]);
	}
	return c;
}

unsigned int getHighestSampleRate(LoggerConfig *config){
   int s = SAMPLE_DISABLED;
   int sr;

   /*
    * Bypass Interval and Utc here since they will always be logging
    * at the highest rate based on the results of this very method
    */

   for (int i = 0; i < CONFIG_ADC_CHANNELS; i++){
      sr = config->ADCConfigs[i].cfg.sampleRate;
      s = getHigherSampleRate(sr, s);
   }

   for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
      sr = config->PWMConfigs[i].cfg.sampleRate;
      s = getHigherSampleRate(sr, s);
   }

   for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
      sr = config->GPIOConfigs[i].cfg.sampleRate;
      s = getHigherSampleRate(sr, s);
   }

   for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
      sr = config->TimerConfigs[i].cfg.sampleRate;
      s = getHigherSampleRate(sr, s);
   }

   for (int i = 0; i < CONFIG_IMU_CHANNELS; i++){
      sr = config->ImuConfigs[i].cfg.sampleRate;
      s = getHigherSampleRate(sr, s);
   }


   GPSConfig *gpsConfig = &(config->GPSConfigs);
   sr = gpsConfig->latitude.sampleRate;
   s = getHigherSampleRate(sr, s);

   sr = gpsConfig->longitude.sampleRate;
   s = getHigherSampleRate(sr, s);

   sr = gpsConfig->speed.sampleRate;
   s = getHigherSampleRate(sr, s);

   sr = gpsConfig->distance.sampleRate;
   s = getHigherSampleRate(sr, s);

   sr = gpsConfig->satellites.sampleRate;
   s = getHigherSampleRate(sr, s);


   LapConfig *trackCfg = &(config->LapConfigs);
   sr = trackCfg->lapCountCfg.sampleRate;
   s = getHigherSampleRate(sr, s);

   sr = trackCfg->lapTimeCfg.sampleRate;
   s = getHigherSampleRate(sr, s);

   sr = trackCfg->sectorCfg.sampleRate;
   s = getHigherSampleRate(sr, s);

   sr = trackCfg->sectorTimeCfg.sampleRate;
   s = getHigherSampleRate(sr, s);

   sr = trackCfg->predTimeCfg.sampleRate;
   s = getHigherSampleRate(sr, s);


   return s;
}

size_t get_enabled_channel_count(LoggerConfig *loggerConfig){
   size_t channels = 0;

   for (size_t i=0; i < CONFIG_TIME_CHANNELS; i++)
      if (loggerConfig->TimeConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
         ++channels;

   for (size_t i=0; i < CONFIG_IMU_CHANNELS; i++)
      if (loggerConfig->ImuConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
         ++channels;

   for (size_t i=0; i < CONFIG_ADC_CHANNELS; i++)
      if (loggerConfig->ADCConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
         ++channels;

   for (size_t i=0; i < CONFIG_TIMER_CHANNELS; i++)
      if (loggerConfig->TimerConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
         ++channels;

   for (size_t i=0; i < CONFIG_GPIO_CHANNELS; i++)
      if (loggerConfig->GPIOConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
         ++channels;

   for (size_t i=0; i < CONFIG_PWM_CHANNELS; i++)
      if (loggerConfig->PWMConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
         ++channels;

   size_t enabled_obd2_pids = loggerConfig->OBD2Configs.enabledPids;
   for (size_t i=0; i < enabled_obd2_pids; i++){
	   if (loggerConfig->OBD2Configs.pids[i].cfg.sampleRate != SAMPLE_DISABLED)
		   ++channels;
   }

   GPSConfig *gpsConfigs = &loggerConfig->GPSConfigs;
   if (gpsConfigs->latitude.sampleRate != SAMPLE_DISABLED) channels++;
   if (gpsConfigs->longitude.sampleRate != SAMPLE_DISABLED) channels++;
   if (gpsConfigs->speed.sampleRate != SAMPLE_DISABLED) channels++;
   if (gpsConfigs->distance.sampleRate != SAMPLE_DISABLED) channels++;
   if (gpsConfigs->satellites.sampleRate != SAMPLE_DISABLED) channels++;


   LapConfig *lapConfig = &loggerConfig->LapConfigs;
   if (lapConfig->lapCountCfg.sampleRate != SAMPLE_DISABLED) channels++;
   if (lapConfig->lapTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;
   if (lapConfig->sectorCfg.sampleRate != SAMPLE_DISABLED) channels++;
   if (lapConfig->sectorTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;
   if (lapConfig->predTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;

   channels += get_virtual_channel_count();
   return channels;
}
