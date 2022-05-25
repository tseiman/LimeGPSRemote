
#include "limegps.h"

#ifndef ControlHttpServer_H
#define ControlHttpServer_H

typedef struct {
    sim_t 	*s;
    double 	gain;
} gps_sim_settings_t;


int runServer(sim_t *s, double gain);

#endif
