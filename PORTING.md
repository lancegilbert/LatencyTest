
### Porting

LatencyTest was loosely built with the concept of being portable to other platforms such as Mac OSX and Linux.

The Qt build environment is intended to work, but not tested, as a unified build system across all platforms, including building directly in Qt Creator.

Additionally, the rtaudio library has been introduced as a dependency in preperation for this task.

Please take note that several pieces of work are preferred prior to serious porting efforts:

```
* The abstractions for platform specific MIDI APIs need to be reviewed and refactored with consideration for CoreMIDI, JACK MIDI, etc.
  - It might be worthwhile to consider dropping the LatencyTest abstractions entirely and using something like [RtMidi](https://github.com/thestk/rtmidi)
* The abstractions for platform specific audio APIs need to be reviewed and refactored with consideration for CoreAudio, JACK, etc.
  - It might be worthwhile to consider dropping the LatencyTest abstractions entirely and using something like [RtAudio](https://github.com/thestk/rtaudio)
* Usage of platform-specific abstractions in non platform specific code (eg: LTMainWindow) needs to be either put in #ifdef blocks (hack) or handled via abstraction (preferable)
```

It is important to note however that any third-party library usage to facilitate timing sensitive work (eg: MIDI or Audio) should be scruitinized for their latency compared to direct usage of the platform API when/if possible.

Care was taken to have minimal layers of indirection in an effort to get a maximally accurate (best-case) latency figure.

This is the reason that they are not currently used, and an original abstraction was attempted. 

However, to my knowledge neither RtMidi or RtAudio have been formally tested for latency compared to direct platform API usage.

It is entirely possible they introduce no relevant latency (API overhead as opposed to additional buffering, etc) in which case they are highly preferable as it would significantly reduce the volume of code and increase compatibility.

The plan is to add RtMidi and RtAudio support using LatencyTest's own platform abstractions (eg: RtAudio as a platform in parallel to ASIO) and use LatencyTest to compare them directly.

If you have additional questions or interests in helping to port or test LatencyTest on other platforms, please reach out to me via any of the methods below!

--
Lance Gilbert
[@lance_gilbert](https://twitter.com/lance_gilbert) on Twitter
[lancegilbertphx](https://www.instagram.com/lancegilbertphx/)  on Instagram
[lancegilbert](https://github.com/lancegilbert) on GitHub

(Updated on March 31st 2017)

