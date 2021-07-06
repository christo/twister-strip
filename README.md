# twister strip

POV LED staff running on ESP32. Currently, building is only tested on MacOS.

* Web interface for choosing effects and speed
* wifi ap created if optionally configured pre-defined access point is inaccessible. intended to be used out in the field without wifi/internet access

# TODO
* try to get makefile working (branch: makefile)
	* tried `arduino-mk` but that doesn't seem to work for `ESP32` if you know it does, let me know and I'll fix it as a PEBCAK.
	* current problem is a compile error due to makeEspArduino using a fancy perl script to find include files but not understanding `gcc` preprocessor directives and includes libs that only compile for ESP8266 which I am not targeting. These are excluded by #if directives and Arduino IDE uses `gcc -E` to figure out the `-I` args. In addition, due to the same reason, the make file takes longer than the IDE, so either I need to explicitly include/exclude header files or instead just use the `arduino` command line utility.
* multiple hardware units coordinating and syncrhonising (over HTTP?)
* gesture controls with IMU
* hardware button or collaborative election of client/server mode which would enable same code to run on each.
	* e.g. look for server, if not found within random timeout, start server, While running server, if no client connects, check for server again, negotiating that server with longest runtime in `millis()` be the server. Or something similar and simpler.
