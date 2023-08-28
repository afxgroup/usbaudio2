USB2Audio

So I included the source, since it's officially "unfinished" and 
I'm too busy with other stuff.

You are welcome to use and modify the code, for non profit uses only.

My coding style may be a bit unusual, but it's hopefully easy to read.

Some of the source files are part of my standard environment. Reusable
code is a big help for old geezers like me.
Here's a breakdown of the source files, and what each is doing for us.

usbaudio2.c
   main() is here, along with support routines that are specific to
   this program. In this case mostly USB support stuff.

actions.c
   three functions, play(), record(), and monitor(); All expect to be
   given a struct AudioDevice that is ready to go.

oca.c (Open Close All)
   handles opening and closing of libraries, devices, and all that stuff.
   also manages command line and tooltype parsing through myargs.c

myargs.c
   parses command line and tooltype options. Called from openAll().

storestring.c
   dynamic string allocation, without length limits.

midisupport.c
   manages all camd.library stuff, used by vumeter_support.

vumetersupport.c
   scans audio data, records peak audio levels, periodically sends
   meter data out to a named CAMD cluster.

wavesupport.c
   reads and writes WAVE format 1 (PCM) audio files.

projectrev.c
   project revision, Version, Screen Title. 
   This should recompile EVERY MAKE
   
makefile
   My environment will update the version string EVERY MAKE.
   This is to support some backup scripts I use regularly.
   The "usual" fake targets are included:
   make > make only files that have changed
   make all > rebuild everything
   make revision > increment version number
   make clean > delete intermediate and executable files


I'm busy, but emails with questions will get usually answers 
within a few days.


Have Fun, make beautiful music.
LyleHaze@gmail.com


