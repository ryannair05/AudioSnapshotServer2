# AudioSnapshotServer2

AudioSnapshotServer hooks AudioUnitRender, which is responsible for rendering audio in Audio Units, and sends the rendered audio data over UDP multicast

## Installation

**iOS 15.0-17.5 required.**

**This tweak by itself does nothing noticeable. A client tweak is neccessary for visible effects - try [Mitsuha Forever](https://github.com/ryannair05/MitsuhaForever/)**

1. Add this repository to Cydia: https://repo.chariz.com.
2. Install AudioSnapshotServer.

## How to use it in your tweak?

Port 44333 at 239.255.0.1 will send a dump of the current audio buffer (raw PCM audio data) every 85ms. That's all to it.
