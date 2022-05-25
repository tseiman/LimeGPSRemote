#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <microhttpd.h>
#include <pthread.h>

#include "ControlHttpServer.h"
#include "gpssim.h"
#include "GPSRunner.h"

#define PAGE "some stuff"

#define OK_FMT  		"{\"type\" : \"OK\", \"param\": \"%s\"}"
#define NOK_FMT 		"{\"type\" : \"FAIL\", \"cause\": \"wrong URL or parametrisation: %s\"}"
#define ALREADY_STARTED_FMT 	"{\"type\" : \"FAIL\", \"cause\": \"GPS SIM already started: %s\"}"
#define ALREADY_STOPPED_FMT 	"{\"type\" : \"FAIL\", \"cause\": \"GPS SIM already stopped: %s\"}"
#define FAIL_START_FMT 		"{\"type\" : \"FAIL\", \"cause\": \"Could not start GPS SIM Thread: %s\"}"

#define MAX_PARAM_LEN 10


static  pthread_t main_thread;



void *run_gps_sim_thread( void *param ) {
    gps_sim_settings_t *settings = (gps_sim_settings_t *)param;
    runGPS(settings->s, settings->gain);

}



static int ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size, void **ptr) {

    static int aptr;
    
    char *errMsg = NOK_FMT;
//    const char *me = cls;
    gps_sim_settings_t *settings = (gps_sim_settings_t *)cls;


    struct MHD_Response *response;
    int ret;


    if (0 != strcmp (method, "GET")) return MHD_NO;              /* unexpected method */
  
    if (&aptr != *ptr) {
      /* do never respond on first call */
      *ptr = &aptr;
      return MHD_YES;
    }
    *ptr = NULL;                  /* reset when done */

//    printf("settings:height: %f, llh0: %f, llh1: %f, llh2: %f,\n", settings->gain, settings->s->opt.llh[0],settings->s->opt.llh[1],settings->s->opt.llh[2]);

    

    printf("URL: %s\n", url);    

    if(0 == strcmp (url, "/start")) {

	const char *param_str;
	char *param_str_END;

	double lat;
	double lon;
	double height;

	if(main_thread) {
	    errMsg = ALREADY_STARTED_FMT;
	    goto FAIL;
	}

	param_str = MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "lat");
	if(strnlen(param_str, MAX_PARAM_LEN) < 1) goto FAIL;
	lat = strtod(param_str, &param_str_END);
	if(lat >180 || lat < -180) goto FAIL;
	if(param_str + strnlen(param_str, MAX_PARAM_LEN)!= param_str_END) goto FAIL;

	param_str = MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "lon");
	if(strnlen(param_str, MAX_PARAM_LEN) < 1) goto FAIL;
	lon = strtod(param_str, &param_str_END);
	if(lon >180 || lon < -180) goto FAIL;
	if(param_str + strnlen(param_str, MAX_PARAM_LEN)!= param_str_END) goto FAIL;

	param_str = MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "height");
	if(strnlen(param_str, MAX_PARAM_LEN) < 1) goto FAIL;
	height = strtod(param_str, &param_str_END);
	if(height >180 || height < -180) goto FAIL;
	if(param_str + strnlen(param_str, MAX_PARAM_LEN)!= param_str_END) goto FAIL;

	settings->s->opt.llh[0] = lat / R2D;
	settings->s->opt.llh[1] = lon / R2D;
	settings->s->opt.llh[2] = height / R2D;
	settings->s->finished = false;

	settings->s->opt.g0.week = -1;
	settings->s->opt.g0.sec = 0.0;
	settings->s->opt.iduration = USER_MOTION_SIZE;
	settings->s->opt.verb = TRUE;
	settings->s->opt.nmeaGGA = FALSE;
	settings->s->opt.staticLocationMode = TRUE;
	settings->s->opt.interactive = FALSE;
	settings->s->opt.timeoverwrite = FALSE;
	settings->s->opt.iono_enable = TRUE;


	int iret =  pthread_create( &main_thread, NULL, run_gps_sim_thread, (void*) settings);
    
	if(iret) {
	    errMsg = FAIL_START_FMT;
	    goto FAIL;
	}
	

	char * response_body = malloc (snprintf (NULL, 0, OK_FMT, url) + 1);
	if (response_body == NULL) return MHD_NO;
	sprintf (response_body, OK_FMT, url);
	response = MHD_create_response_from_buffer (strlen (response_body), response_body,  MHD_RESPMEM_MUST_FREE);
        MHD_add_response_header (response, "Content-Type", "application/json");
	if (response == NULL) {
    	    free(response_body);
    	    return MHD_NO;
	}
	ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);
	return ret;
    } else if(0 == strcmp (url, "/stop")) {

	if(!main_thread) {
	    errMsg = ALREADY_STOPPED_FMT;
	    goto FAIL;
	}
	printf("terminating %lu\n",main_thread);
//	settings->s->finished = true;
	pthread_kill(main_thread,SIGINT);
	pthread_kill(main_thread,SIGTERM);
// pthread_cancel(main_thread);
	pthread_join(main_thread, NULL);
	main_thread = 0;
	printf("Finished simulation\n");
	char * response_body = malloc (snprintf (NULL, 0, OK_FMT, url) + 1);
	if (response_body == NULL) return MHD_NO;
	sprintf (response_body, OK_FMT, url);
	response = MHD_create_response_from_buffer (strlen (response_body), response_body,  MHD_RESPMEM_MUST_FREE);
        MHD_add_response_header (response, "Content-Type", "application/json");
	if (response == NULL) {
    	    free(response_body);
    	    return MHD_NO;
	}
	ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);
	return ret;
    }
    

FAIL:
    {
	char * response_body = malloc (snprintf (NULL, 0, errMsg, url) + 1);
	if (response_body == NULL) return MHD_NO;
	sprintf (response_body, errMsg, url);
	response = MHD_create_response_from_buffer (strlen (response_body), response_body,  MHD_RESPMEM_MUST_FREE);
        MHD_add_response_header (response, "Content-Type", "application/json");
	if (response == NULL) {
	    free(response_body);
    	    return MHD_NO;
	}
        ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);
	return ret;
    }
}


int runServer(sim_t *s, double gain) {
    struct MHD_Daemon *d;

    gps_sim_settings_t settings;
    settings.s = s;
    settings.gain = gain;

    main_thread = 0;

    d = MHD_start_daemon (// MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG | MHD_USE_POLL,
	    MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
	    // MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG | MHD_USE_POLL,
	    // MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG,
                        8080,
                        NULL, NULL, &ahc_echo, &settings,
	    MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
	    MHD_OPTION_END);
    if (d == NULL) return 1;
    
    char x;
    while (x != 'q') {
	printf("(q)uit?\n");
	x = getc(stdin);
    }
    MHD_stop_daemon (d);
    
    return 0;
}
