# MTPlayer
A MONOTONE-compatible player library written in C.

## What is MONOTONE?
MONOTONE is a tracker (song editor), kind of like [OpenMPT](https://openmpt.org/).
However, unlike OpenMPT, which is (mostly) sample-based, MONOTONE is aimed for much simpler audio devices,
it only supports one kind of instrument (a square wave) at a constant volume.
Its music file format (.MON files) is usually only a couple of kB in size.

This library is (mostly) just a translation of the original MONOTONE tracker source code (in Pascal) to C. It includes a simple example program, which loads and plays a .MON file using PortAudio.

For more info about MONOTONE & example .MON files, please visit the [MONOTONE GitHub repo](https://github.com/MobyGamer/MONOTONE).
