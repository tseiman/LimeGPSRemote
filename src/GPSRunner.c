
#include <signal.h>
#include "limegps.h"


static sim_t *s;
static lms_device_t *device = NULL;
static int32_t channel = 0;


void sig_handler_gpsRunner(int signum){
    printf("Catched Signal in GPSRunner\n");
    s->finished = true;
//    pthread_kill(s->tx.thread, SIGKILL);
	LMS_StopStream(&s->tx.stream);
	LMS_DestroyStream(device, &s->tx.stream);
	// Free up resources
	if (s->tx.buffer != NULL)
		free(s->tx.buffer);
	if (s->fifo != NULL)
		free(s->fifo);

	LMS_EnableChannel(device, LMS_CH_TX, channel, false);
	LMS_Close(device);

//     pthread_exit(0);
}


int runGPS(sim_t *setting, double gain) {

    s = setting;

    signal(SIGTERM,sig_handler_gpsRunner); 

    datetime_t t0;

    s->opt.timeoverwrite = TRUE;
    time_t timer;
    struct tm *gmt;
    time(&timer);
    gmt = gmtime(&timer);

    t0.y = gmt->tm_year+1900;
    t0.m = gmt->tm_mon+1;
    t0.d = gmt->tm_mday;
    t0.hh = gmt->tm_hour;
    t0.mm = gmt->tm_min;
    t0.sec = (double)gmt->tm_sec;

    date2gps(&t0, &s->opt.g0);




	// Find device
	int device_count = LMS_GetDeviceList(NULL);

	if (device_count < 1)
	{
		printf("ERROR: No device was found.\n");
		exit(1);
	}
	else if (device_count > 1)
	{
		printf("ERROR: Found more than one device.\n");
		exit(1);
	}

	lms_info_str_t *device_list = malloc(sizeof(lms_info_str_t) * device_count);
	device_count = LMS_GetDeviceList(device_list);

	// Initialize simulator
	init_sim(s);

	// Allocate TX buffer to hold each block of samples to transmit.
	s->tx.buffer = (int16_t *)malloc(SAMPLES_PER_BUFFER * sizeof(int16_t) * 2); // for 16-bit I and Q samples
	
	if (s->tx.buffer == NULL) 
	{
		printf("ERROR: Failed to allocate TX buffer.\n");
		goto out;
	}

	// Allocate FIFOs to hold 0.1 seconds of I/Q samples each.
	s->fifo = (int16_t *)malloc(FIFO_LENGTH * sizeof(int16_t) * 2); // for 16-bit I and Q samples

	if (s->fifo == NULL) 
	{
		printf("ERROR: Failed to allocate I/Q sample buffer.\n");
		goto out;
	}

	// Initializing device
	printf("Opening and initializing device...\n");

//	lms_device_t *device = NULL;

	if (LMS_Open(&device, device_list[0], NULL))
	{
		printf("ERROR: Failed to open device: %s\n", device_list[0]);
		goto out;
	}

	const lms_dev_info_t *devinfo =  LMS_GetDeviceInfo(device);

	if (devinfo == NULL)
	{
		printf("ERROR: Failed to read device info: %s\n", LMS_GetLastErrorMessage());
		goto out;
	}

    printf("deviceName: %s\n", devinfo->deviceName);
    printf("expansionName: %s\n", devinfo->expansionName);
    printf("firmwareVersion: %s\n", devinfo->firmwareVersion);
    printf("hardwareVersion: %s\n", devinfo->hardwareVersion);
    printf("protocolVersion: %s\n", devinfo->protocolVersion);
    printf("gatewareVersion: %s\n", devinfo->gatewareVersion);
    printf("gatewareTargetBoard: %s\n", devinfo->gatewareTargetBoard);

    int limeOversample = 1;
    if(strncmp(devinfo->deviceName, "LimeSDR-USB", 11) == 0)
    {
        limeOversample = 0;    // LimeSDR-USB works best with default oversampling
        printf("Found a LimeSDR-USB\n");
    }
    else
    {
        printf("Found a LimeSDR-Mini\n");
    }

	int lmsReset = LMS_Reset(device);
	if (lmsReset)
	{
		printf("ERROR: Failed to reset device: %s\n", LMS_GetLastErrorMessage());
		goto out;
	}

	int lmsInit = LMS_Init(device);
	if (lmsInit)
	{
		printf("ERROR: Failed to linitialize device: %s\n", LMS_GetLastErrorMessage());
		goto out;
	}

	// Select channel
//	int32_t channel = 0;
	//int channel_count = LMS_GetNumChannels(device, LMS_CH_TX);

	// Select antenna
	//int32_t antenna = 1;
	int antenna_count = LMS_GetAntennaList(device, LMS_CH_TX, channel, NULL);
	lms_name_t *antenna_name = malloc(sizeof(lms_name_t) * antenna_count);

	if (antenna_count > 0)
	{
		int i = 0;
		lms_range_t *antenna_bw = malloc(sizeof(lms_range_t) * antenna_count);
		LMS_GetAntennaList(device, LMS_CH_TX, channel, antenna_name);
		for (i = 0; i < antenna_count; i++)
		{
			LMS_GetAntennaBW(device, LMS_CH_TX, channel, i, antenna_bw + i);
			//printf("Channel %d, antenna [%s] has BW [%lf .. %lf] (step %lf)" "\n", channel, antenna_name[i], antenna_bw[i].min, antenna_bw[i].max, antenna_bw[i].step);
		}
	}

	LMS_SetNormalizedGain(device, LMS_CH_TX, channel, gain);
	// Disable all other channels
	LMS_EnableChannel(device, LMS_CH_TX, 1 - channel, false);
	LMS_EnableChannel(device, LMS_CH_RX, 0, false);
	LMS_EnableChannel(device, LMS_CH_RX, 1, false);
	// Enable our Tx channel
	LMS_EnableChannel(device, LMS_CH_TX, channel, true);

	int setLOFrequency = LMS_SetLOFrequency(device, LMS_CH_TX, channel, (double)TX_FREQUENCY);
	if (setLOFrequency)
	{
		printf("ERROR: Failed to set TX frequency: %s\n", LMS_GetLastErrorMessage());
		goto out;
	}

	// Set sample rate
	lms_range_t sampleRateRange;
	int getSampleRateRange = LMS_GetSampleRateRange(device, LMS_CH_TX, &sampleRateRange);
	if (getSampleRateRange)
		printf("Warning: Failed to get sample rate range: %s\n", LMS_GetLastErrorMessage());

	int setSampleRate = LMS_SetSampleRate(device, (double)TX_SAMPLERATE, limeOversample); 

	if (setSampleRate)
	{
		printf("ERROR: Failed to set sample rate: %s\n", LMS_GetLastErrorMessage());
		goto out;
	}

	double actualHostSampleRate = 0.0;
	double actualRFSampleRate = 0.0;
	int getSampleRate = LMS_GetSampleRate(device, LMS_CH_TX, channel, &actualHostSampleRate, &actualRFSampleRate);
	if (getSampleRate)
		printf("Warnig: Failed to get sample rate: %s\n", LMS_GetLastErrorMessage());
	else
		printf("Sample rate: %.1lf Hz (Host) / %.1lf Hz (RF)" "\n", actualHostSampleRate, actualRFSampleRate);

	// Automatic calibration
	printf("Calibrating...\n");
	int calibrate = LMS_Calibrate(device, LMS_CH_TX, channel, (double)TX_BANDWIDTH, 0);
	if (calibrate)
		printf("Warning: Failed to calibrate device: %s\n", LMS_GetLastErrorMessage());

	// Setup TX stream
	printf("Setup TX stream...\n");
	s->tx.stream.channel = channel;
	s->tx.stream.fifoSize = 1024 * 1024;
	s->tx.stream.throughputVsLatency = 0.5;
	s->tx.stream.isTx = true;
	s->tx.stream.dataFmt = LMS_FMT_I12;
	int setupStream = LMS_SetupStream(device, &s->tx.stream);
	if (setupStream)
	{
		printf("ERROR: Failed to setup TX stream: %s\n", LMS_GetLastErrorMessage());
		goto out;
	}

	// Start TX stream
	LMS_StartStream(&s->tx.stream);

	// Start GPS task.
	s->status = start_gps_task(s);
	if (s->status < 0) {
		fprintf(stderr, "Failed to start GPS task.\n");
		goto out;
	}
	else
		printf("Creating GPS task...\n");

	// Wait until GPS task is initialized
	pthread_mutex_lock(&(s->tx.lock));
	while (!s->gps.ready)
		pthread_cond_wait(&(s->gps.initialization_done), &(s->tx.lock));
	pthread_mutex_unlock(&(s->tx.lock));

	// Fillfull the FIFO.
	if (is_fifo_write_ready(s))
		pthread_cond_signal(&(s->fifo_write_ready));

	// Start TX task
	s->status = start_tx_task(s);
	if (s->status < 0) {
		fprintf(stderr, "Failed to start TX task.\n");
		goto out;
	}
	else
		printf("Creating TX task...\n");

	// Running...
#ifdef WIN32
	printf("Running...\n" "Press 'q' to abort.\n");
#else
	printf("Running...\n" "Press Ctrl+C to abort.\n");
#endif

	// Wainting for TX task to complete.
	pthread_join(s->tx.thread, NULL);
	printf("\nDone!\n");

out:
	// Disable TX module and shut down underlying TX stream.
	LMS_StopStream(&s->tx.stream);
	LMS_DestroyStream(device, &s->tx.stream);

	// Free up resources
	if (s->tx.buffer != NULL)
		free(s->tx.buffer);

	if (s->fifo != NULL)
		free(s->fifo);

	printf("Closing device...\n");
	LMS_EnableChannel(device, LMS_CH_TX, channel, false);
	LMS_Close(device);

}
