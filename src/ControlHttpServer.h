
#include "limegps.h"

#ifndef ControlHttpServer_H
#define ControlHttpServer_H


#define THREAD_RESULT_WAIT 		-254
#define THREAD_RESULT_OK  		0
#define THREAD_RESULT_SDR_ERR  		1
#define THREAD_RESULT_BUFFER_ERR  	2
#define THREAD_RESULT_TASK_ERR  	3



typedef struct {
    int 	returnval;
    char 	*msg;
} thread_result_t;

typedef struct {
    sim_t 	*s;
    double 	gain;
    thread_result_t *result;
} gps_sim_settings_t;


int runServer(sim_t *s, double gain, char *tlsKeyFile, char *tlsCertFile);

#endif
