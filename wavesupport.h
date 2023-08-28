#ifndef WAVE_SUPPORT_H
#define WAVE_SUPPORT_H

// wave_support.h
// simple create/load/save of audio ?#?.wav files
// see head of wave_support.c for usage help

struct WaveFile
{
   struct MinNode wf_Node;
   STRPTR         wf_FileName;
   BPTR           wf_FileHandle;
   uint32         wf_FmtSize;
   uint32         wf_ChannelCount;
   uint32         wf_SampleRate;
   uint32         wf_SampleWidth;
   uint32         wf_AudioFormat;
   uint32         wf_DataSize;
   APTR           wf_AudioData;
};

// offsets for WAV file header entries
#define WAVE_FileSize        4 // actual-8 , 4 bytes
#define WAVE_Format         20 // sound format, 1 byte
#define WAVE_ChannelCount   22 // 2 bytes
#define WAVE_SampleRate     24 // 4 bytes
#define WAVE_BytesPerSecond 28 // rate*bitper*channels/8 4 bytes
#define WAVE_BytesPerPacket 32 // bitsper*channels/8 2 bytes
#define WAVE_BitsPerSample  34 // 2 bytes
#define WAVE_DataSize       40 // data section only, 4 bytes


/* creates a new WaveFile structure,
 * opens the named file, deleting any previous file contents.
 * Writes a blank wave file header(will be filled in later)
 * returns file open and ready for writing audio data.
 * call finishWave() later to wrap things up.
 */
struct WaveFile *newWave(STRPTR filename);

/* Formats and prints any error message given.
 * Closes any open FileHandle,
 * frees the WaveFile structure, and all contents
 * always returns NULL
 */
struct WaveFile *freeWave(struct WaveFile *wave, CONST_STRPTR reason, ...);

/* Open existing WAVE file
 * create struct WaveFile, read and parse WAVE header into struct
 * if buffered, also allocate and read the audio data into memory.
 * non-NULL return is pointer to new struct WaveFile, completed.
 * if buffered, the audio data has already been allocated and filled,
 * and the file handle has been closed.
 * if not buffered, wf_FileHandle is ready to read audio data
 * if return NULL, all resources are already freed.
 */
struct WaveFile *loadWave(STRPTR fileName, BOOL buffered);
#define WAVE_BUFFERED TRUE
#define WAVE_FROM_DISK FALSE


/* after saving AudioData, fill out the WaveFile members
 * wf_AudioFormat (probably 1)
 * wf_ChannelCount, wf_SampleRate, wf_SampleWidth to correct values,
 * then call here to finalize and close the file
 */
BOOL finishWave(struct WaveFile *wave);

// corrects 24 bits to 32, actual container width
uint32 byteWidth(uint32 sampleWidth);

// read nBytes from file, with endian swap
// returns value, or -1 for EOF
int32 readLE(BPTR file, uint32 nBytes);

// seek a 4 byte sequence from current position in the open file.
// returns TRUE with file position immediately after if found
// returns FALSE with file position where it was if not found
BOOL seek4(BPTR file, uint32 c0, uint32 c1, uint32 c2, uint32 c3);

#endif

