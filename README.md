# LatencyTest

A tool for approximating MIDI and synthesesizer latency

### Downloading & Installing (Users)

Installer builds of LatencyTest will be made available soon. Please check back soon!

### Getting Started (Developers)

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Platform Notes and Prerequisites

LatencyTest works out of the box for users, but has a few caveats and pre-requisites for developers!

Since LatencyTest is built using Qt 5.8.0, it is possible to use Qt Creator/qmake to generate build scripts for a great deal of platforms.

However, at this time LatencyTest has been built and tested only on Windows 10, 64bit using Visual Studio 2015.

For information regarding getting LatencyTest building other platforms such as OSX or Linux see PORTING.md!

To compile and run LatencyTest you will need the following:

```
* Qt 5.8.0 (msvc2015_64, opensource)
* Visual Studio 2015
* ASIO 2.3 SDK (Run getasiosdk.bat)
* rtaudio (Run getsubmodules.bat)
```
### Building and Running LatencyTest

TODO

### Generating Installer

TODO

### Built With

* Qt 5.8 [Open Source Edition](https://www.qt.io/download-open-source/) - Lesser GPL Version 3
* ASIO SDK 2.3 [Steinberg](https://www.steinberg.net/en/company/developers.html)
* RtAudio [RtAudio](https://github.com/thestk/rtaudio) - MIT Style License

### Contributing

A contributing guide will be established soon.

### Authors

* **Lance Gilbert** - *Initial work* - [lancegilbert](https://github.com/lancegilbert)

### License

This project is licensed under the GPL Version 3
