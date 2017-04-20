
### Porting

LatencyTest was loosely built with the concept of being portable to other platforms such as Mac OSX and Linux.

The Qt build environment is intended to work, but not tested, as a unified build system across all platforms, including building directly in Qt Creator.

Additionally, the rtaudio library has been introduced as a dependency in preperation for this task.

Please take note that several pieces of work are preferred prior to serious porting efforts:

```
* The MIDI API abstraction needs to be reviewed/refactored with consideration for CoreMIDI, JACK MIDI, etc.
  - Consider dropping our abstractions entirely in favor of [RtMidi](https://github.com/thestk/rtmidi)
* The audio API abstraction needs to be reviewed/refactored with consideration for CoreAudio, JACK, etc.
  - Consider dropping our abstractions entirely in favor of [RtAudio](https://github.com/thestk/rtaudio)
* Clean up usage of platform-specific classes in common code like LTMainWindow.
```

It is important to note however that any third-party libraries used to for timing sensitive work (eg: MIDI or Audio) should have their latency analyzed and compared to direct "optimal" usage of the low-level platform API when/if possible.

Native implementations were done in favor of abstractions such as RtMidi/RtAudio in an effort to get a maximally accurate (best-case) latency figure.

Care was taken to avoid buffering or indirection to whatever degree feasible.

With that said, I have not yet tested either RtMidi or RtAudio for latency compared to direct platform API usage.

It is entirely possible they introduce no relevant latency in which case they are highly preferable as it would significantly reduce the volume of code while also making porting much simpler.

The plan is to add RtMidi and RtAudio support using LatencyTest's own platform abstractions (eg: RtAudio as a platform in parallel to ASIO) and use LatencyTest to compare them directly.

If you have additional questions or interests in helping to port or test LatencyTest on other platforms, please reach out to me via any of the methods below!

--
Lance Gilbert
[@lance_gilbert](https://twitter.com/lance_gilbert) on Twitter
[lancegilbertphx](https://www.instagram.com/lancegilbertphx/)  on Instagram
[lancegilbert](https://github.com/lancegilbert) on GitHub

(Updated on April 4th 2017)

