#define _CRT_SECURE_NO_WARNINGS

#include "limegps.h"
#include <math.h>
#include <signal.h>
#include <unistd.h>

// for _getch used in Windows runtime.
#ifdef WIN32
#include <conio.h>
#include "getopt.h"
#else
#include <unistd.h>
#endif


#include "limegps.h"
#include "ControlHttpServer.h"


void sig_handler(int signum){
  printf("Catched Signal\n");
}

void usage(char *progname)
{
	printf("Usage: %s [options]\n"
		"Options:\n"
		"  -e <gps_nav>     RINEX navigation file for GPS ephemerides (required)\n"
		"  -a <rf_gain>     Normalized RF gain in [0.0 ... 1.0] (default: 0.1)\n",
		progname);
	return;
}


int main(int argc, char *argv[]) {
    if (argc<3) {
	usage(argv[0]);
	exit(1);
    }

    signal(SIGINT,sig_handler); 

    // Set default values
    sim_t s;

    s.finished = false;
    s.opt.navfile[0] = 0;
    s.opt.umfile[0] = 0;
    s.opt.g0.week = -1;
    s.opt.g0.sec = 0.0;
    s.opt.iduration = USER_MOTION_SIZE;
    s.opt.verb = TRUE;
    s.opt.nmeaGGA = FALSE;
    s.opt.staticLocationMode = TRUE;
    //s.opt.llh[0] = 35.6811673 / R2D;
    //s.opt.llh[1] = 139.7648576 / R2D;
    s.opt.llh[0] = 40.7850916 / R2D;
    s.opt.llh[1] = -73.968285 / R2D;
    s.opt.llh[2] = 10.0;
    s.opt.interactive = FALSE;
    s.opt.timeoverwrite = FALSE;
    s.opt.iono_enable = TRUE;


    // Options
    int result;
//	double duration;
//    datetime_t t0;
    double gain = 0.1;

    while ((result=getopt(argc,argv,"e:a:"))!=-1) {
	switch (result) {
	    case 'e':
		strcpy(s.opt.navfile, optarg);
		break;
	    case 'a':
		gain = atof(optarg);
		if (gain < 0.0) gain = 0.0;
		if (gain > 1.0) gain = 1.0;
		break;
	    case ':':
	    case '?':
		usage(argv[0]);
		exit(1);
	    default:
		break;
	}
    }

    if (s.opt.navfile[0]==0) {
	printf("ERROR: GPS ephemeris file is not specified.\n");
	exit(1);
    }

//    runGPS(&s, gain);
    runServer(&s, gain);
    return(0);
}

