/*
 * sampleRecord.h
 *
 *  Created on: Feb 29, 2012
 *      Author: brent
 */

#ifndef SAMPLERECORD_H_
#define SAMPLERECORD_H_

#include <limits.h>
#include "loggerConfig.h"

#define NIL_SAMPLE INT_MIN + 1

#define LOGGER_MSG_SAMPLE    0
#define LOGGER_MSG_START_LOG 1
#define LOGGER_MSG_END_LOG   2


typedef struct _ChannelSample {
   unsigned short channelId;
   unsigned short sampleRate;
   size_t channelIndex;
   float (*get_sample)(int);
   union {
      int intValue;
      float floatValue;
      long long llValue;
   };
} ChannelSample;

typedef struct _LoggerMessage
{
	int messageType;
	size_t sampleCount;
	ChannelSample * channelSamples;
} LoggerMessage;

ChannelSample * create_channel_sample_buffer(LoggerConfig *loggerConfig, size_t channelCount);

#endif /* SAMPLERECORD_H_ */
