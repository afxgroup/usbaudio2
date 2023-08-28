USBAudio2 53.1 (08/20/2023)
©20.8.2023 Lyle Hazelwood, all rights reserved

USB Audio2 record and playback
Records and plays back audio over USB to a select type of USB audio devices.
Also supports VU Meters over CAMD.

Command Line driven, NO GUI at this time.

All source included, both as an example of Isochronous USB transfers, and 
so that users can modify, improve, and further develop the program.


USBAudio ?
FILE,RECORD/S,PLAY/S,MONITOR/S,SECONDS/N,CHANNELS/N,RATE/N,WIDTH/N,
RAWFKT/K,METER/K,MINIOR/K,ALTERNATE/N,FILLOUT/S,HELP/S,V=VERBOSE/S

FILE: required filename for RECORD and PLAY. 

RECORD:  \
PLAY:     \
MONITOR:   \ ONE (only one) of these three is required

Record: opens the USB audio device and records audio to FILE.
   Records at the default rate of your device, or as close to RATE as possible.
   Records the default number of channels of your device, or CHANNELS.
   Continues to record until Ctrl-C, BREAK, or until SECONDS have passed.
   File saved in wave format 1 (PCM).
   Input levels displayed to VU meters at camd cluster METER if provided.
   Displays lots of unnecessary crap if VERBOSE is selected.
   Displays help text if HELP is selected.
   Will try to open a provided RAWFKT if a hex address is provided.(untested)
   Caller may increase the number of IO Requests used with MINIOR.
   Caller may specify a USB ALTERNATE if desired. (default 1, see below)

   The WIDTH argument is currently ignored.

Play: opens the USB audio device and plays audio from FILE.
   Plays as close to rate specified in the file as possible.
   Plays the number of channels in the file, or more if FILLOUT specified.
   Continues to play until end of file, Ctrl-C, BREAK, or until SECONDS have passed.
   File must be in wave format 1 (PCM).
   Output levels displayed to VU meters at camd cluster METER if provided.
   Displays lots of unnecessary crap if VERBOSE is selected.
   Displays help text if HELP is selected.
   Will try to open a provided RAWFKT if a hex address is provided.(untested)
   Caller may increase the number of IO Requests used with MINIOR.
   Caller may specify a USB ALTERNATE if desired. (default 1, see below)

   The WIDTH argument is currently ignored.

Monitor: Displays input levels on VU meters without recording.
   Much like Record, except no filename required, no recording is done.
   It's for checking input levels before recording.
   Everything else is just like Record command.
   
RATE: will default to your devices default, probably 48000. If specified the program
   will submit your request,and whatever your device actually does is what you get.

RAWFKT: This was added so an external GUI can specify which USB device to use.
   it is untested. The arg is passed in as a hexadecimal number. 
   IUSB->USBFindFunction() is used to find the function. If you use this feature, 
   you are responsible for USBUnlockFunction() after use.
   When not specified, the program will use the first suitable Audio2 device found.

METER: specifies the name of the CAMD cluster where your VU meter is located.
   see the VUMeter documents for more information.

MINIOR: allows the user to increase the number of IORequests for this operation.
   More requests may help with dropped USB data.

ALTERNATE: Allows the user to specify which "alternate" setting we are working
   with. Some USB Audio devices have different alternates for different bandwidth
   requirements, or different sample widths, or different sample rates, or whatever.
   A "proper" driver would search all alternates, but I have not done this yet.
   Alternate 0 is ALWAYS "Off", releasing all bandwidth. This is always done on exit.

FILLOUT: On PLAY only, if the source FILE has less channels than the output device,
   this will copy the source channels as many times as dest channels can take.
   This will make a monophonic (1 channel) file play back on both channels of a 
   Stereo(2 channel) output device. It also makes stereo files play back on both
   sets of outputs on my 4 channel USB audio device.

HELP: Prints a bunch of help about how to use this program.

VERBOSE: prints a bunch of useless junk about how it's doing the task.



About the "Audio2" requirement:

   The USB standard for Audio playback and recording originally left out a
   few details, like how to set and get the sample rate. As a result, each
   vendor did things a bit differently, and none of this is documented.

   Later, they revised the standard, and USB Audio 2 was defined. The new
   standard changed enough stuff that there is NO BACKWARDS COMPATIBILITY.
   
   So, "Audio2" is a revised protocol. This is NOT "USB2", that's something 
   entirely different. You can tell by looking at the id_Protocol field of
   the Interface descriptor. I'll post a separate executable that you can
   use to test devices you may have.

   This program SHOULD work with most/all Audio2 devices. Audio1 is not
   supported here, and if I ever do support it, it will be in a separate
   program.


Missing Features:
   There's a HUGE amouint of stuff the standard supports, and every time I 
   try to add these things, I get lost in the forest. That's why this is so
   basic. Record, Play, Start, Stop. Shell driven so someone COULD add a GUI
   later if desired. Source code included in case someone wants to add
   features.
   Basic WAVE type 1 only (PCM), because that's what the devices use.
   There's a billion more "easy" features like multi-format support, resampling,
   mixing, supporting built-in features, .. the list goes on. If I try to add
   these myself (I already did some) I'll never finish. So It's released like
   it is, and hopefully can be a starting place for others.
   
   
Multi-Channel support:
   My device records and plays back four channels at a time, A friend has
   an eight channel device. The WAVE format supports multiple channels, but
   most other software only works with one or two channels. If your 
   recordings won't play back on other programs, try recording just one or 
   two channels.
   
This software was tested using a Behringer UMC404HD, which records and plays
   four channels at up to 192Khz. I expect it will probably work with the
   the smaller UM devices as well(UMC202HD, UMC22, UM2) but that's untested.
   It SHOULD work with other Audio2 devices, please let me know.


Source code is included. It's not perfect, but it's right here if you want 
to fix it. I use a lot of comments. My coding style is a bit unusual, but 
hopefully easy to figure out. I might add a separate readme for programmers.

Recommended documents include the USB "Audio20 final.pdf", along with the
usual USB documents.


I'd love to hear any comments you have. I'd love to get a copy of any changes
you make or features you add. I'd love to see a GUI for this thing.

I'd love to see the isochronous code applied to FFMPEG for a webcam viewer.
Every time I get into FFMPEG I get lost again. I get lost a lot.


LyleHaze@gmail.com

