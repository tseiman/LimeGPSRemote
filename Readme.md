# LimeGPSRemote

Based on [LimeGPS](https://github.com/osqzss/LimeGPS) which is based on [gps-sdr-sim](https://github.com/osqzss/gps-sdr-sim) for real-time GPS signal generation.
The code works with LimeSDR and has been tested on Ubuntu 18.04. The default TX antenna port is TX1_1. In Opposite to LimGPS this version has a little less options.
Starts time generation always from now. No Dynamic mode or any motion simulation.
Instead this version allows to be remote controlled via HTTP (no security!!).

http://myhost.localdomain:8080/start?lon=45.23456&lat=11.23456&alt=100  e.g. starts the generation with given Latitude, Longitude and Altitude. 
http://myhost.localdomain:8080/stop stops the generation. Stop must be executed before new start. 

```
Usage: LimeGPSServer [options]
Options:
  -e <gps_nav>     RINEX navigation file for GPS ephemerides (required)
  -a <rf_gain>     Normalized RF gain in [0.0 ... 1.0] (default: 0.1)
```

The server is not deamonizing and can be stopped by press [q]<ENTER>


### Build on Linux (Ubuntu 18.04)

1. Install LimeSuite and all the MyriadRF drivers.

 ```
$ sudo add-apt-repository -y ppa:myriadrf/drivers
$ sudo apt-get update
$ sudo apt-get install limesuite liblimesuite-dev limesuite-udev limesuite-images
$ sudo apt-get install soapysdr soapysdr-module-lms7
$ sudo apt-get install libmicrohttpd-dev
```

2. Clone and build LimeGPSRemote.

 ```
$ git clone https://github.com/tseiman/LimeGPSRemote
$ cd LimeGPSRemote
$ cmake .
$ make
```

3. Test the simulator.

 ```
$ ./LimeGPSServer -e brdc0350.18n
```


