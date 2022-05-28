
#include "ControlHttpServer.h"

#ifndef GPSRunner_H
#define GPSRunner_H

#define ERROR_RESULT(RETURNVALUE, MESSAGE) \
		result->msg = MESSAGE; \
		result->returnval = RETURNVALUE; \
		printf("ERROR: %s\n", result->msg);

#define OK_RESULT \
		result->msg = "OK"; \
		result->returnval = THREAD_RESULT_OK;

void runGPS(sim_t *s, double gain, thread_result_t *result); 

#endif