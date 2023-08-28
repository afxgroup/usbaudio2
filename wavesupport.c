#include <stdarg.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include "wavesupport.h"
#include "usbsupport.h"
#include "myargs.h"
#include "storestring.h"
#include "oca.h"

/*
 *	Procedure to create a NEW wave file
 *		struct WaveFile *wave 
 *    if(wave = newWave("filename"))
 *		{
 *			IDOS->FWrite(wave->wf_FileHandle, audio, sizeof(audio),1);
 *			// describe the audio format you wrote
 *			wave->wf_AudioFormat  = 1;	// 1 is LPCM, almost always correct (default)
 *			wave->wf_SampleRate   = ?;	// 48000 is popular
 *			wave->wf_ChannelCount = ?;	// 1 and 2 are popular, we support more!
 *			wave->wf_SampleWidth  = ?;	// 16 and 32 are common
 *			finishWave(wave);
 *			// file is closed, but structure and data are all available
 *       wave = freeWave(wave, NULL);
 *       // all resources freed, wave == NULL;
 *		}
 */


/*
 *	Procedure to load and play wave file from disk
 *		struct WaveFile *wave 
 *    if(wave = loadWave("filename", WAVE_FROM_DISK))
 *		{
 *			// configure playback for wave->wf_* details
 *			wave->wf_AudioFormat  = 1;	// 1 is LPCM, almost always correct (default)
 *			wave->wf_SampleRate   = ?;
 *			wave->wf_ChannelCount = ?;
 *			wave->wf_SampleWidth  = ?; // in bits
 *
 *			while(bytes=IDOS->FRead(wave->wf_FileHandle, buffer, 1, sizeof(buffer)))
 *			{
 *				// play bytes from buffer
 *			}
 *
 *			wave = freeWave(wave);
 *       // all resources freed, wave == NULL;
 *		}
 */

/*
 *	Procedure to load and play wave file from memory
 *		struct WaveFile *wave 
 *    if(wave = loadWave("filename", WAVE_BUFFERED))
 *		{
 *			// configure playback for wave->wf_* details
 *			wave->wf_AudioFormat  = 1;	// 1 is LPCM, almost always correct (default)
 *			wave->wf_SampleRate   = ?;
 *			wave->wf_ChannelCount = ?;
 *			wave->wf_SampleWidth  = ?; // in bits
 *
 *			Audio data is at wave->wf_AudioData, length is wave->wf_DataSize;
 *
 *			wave = freeWave(wave);
 *       // all resources freed, wave == NULL;
 *		}
 */





static void writeLE32(
	BPTR, int32 offset, uint32 value);
static void writeLE16(
	BPTR, int32 offset, uint16 value);
static void writeLE8 (
	BPTR, int32 offset, uint8 value);

static uint8 waveHeaderData[]=
{
	'R','I','F','F',
	0,0,0,0,				// total file size - 8
	'W','A','V','E',
	'f','m','t',32,
	16,0,0,0,			// size of format section, 16
	1,0,					// sample format
	2,0,					// number of channels
	0,0,0,0,				// sample rate
	0,0,0,0,				// (rate*bitsPerSample*channels)/8
	4,0,					// (bitspersample*channels)/ 8
	0,0,					// bitspersample
	'd','a','t','a',
	0,0,0,0				// file size of data section
};

/* creates a new WaveFile structure,
 * opens the named file, deleting any previous file contents.
 * Writes a blank wave file header(will be filled in later)
 * returns file open and ready for writing data.
 * call finishWave() later to wrap things up.
 */
struct WaveFile *newWave(STRPTR filename)
{
	struct WaveFile *wave;
	
	if(!(wave = IExec->AllocVecTags(
		sizeof(struct WaveFile),
		AVT_ClearWithValue,0,
		TAG_END)))
	{
		return(freeWave(wave, "newWave failed to allocate a WaveFile structure\n"));
	}

	storeString(&wave->wf_FileName, filename);

	wave->wf_FileHandle = IDOS->FOpen(wave->wf_FileName, MODE_NEWFILE, 0);
	if(0 == wave->wf_FileHandle)
	{
		return(freeWave(wave, "newWave(%s) failed to open file\n", 
			wave->wf_FileName));
	}

	if(!(IDOS->FWrite(wave->wf_FileHandle, waveHeaderData, sizeof(waveHeaderData),1)))
	{
		return(freeWave(wave, "newWave(%s) failed to write WAVE header\n", 
			wave->wf_FileName));	
	}

	return(wave);
}

/* Formats and prints any error message given.
 * Closes any open FileHandle,
 * frees the WaveFile structure, and all contents
 * always returns NULL
 */
struct WaveFile *freeWave(struct WaveFile *wave, CONST_STRPTR reason, ...)
{
	if(reason)
	{
		uint32 args[8], x;
		va_list ap;
		va_start(ap, reason);
		for(x = 0; x < 8; x++)
		{
			args[x] = va_arg(ap, uint32);
		}
		va_end(ap);
		errMessage(reason, args[0], args[1], args[2], args[3], 
									args[4], args[5], args[6], args[7]);
	}

	if(!wave)
	{
		return(NULL);
	}

	if(wave->wf_FileHandle)
	{
		IDOS->FClose(wave->wf_FileHandle);
	}

	IExec->FreeVec(wave->wf_AudioData);

	IExec->FreeVec(wave);

	return(NULL);
}

/* Open existing WAVE file
 * create struct WaveFile, read and parse WAVE header into struct
 * if buffered, also allocate and read the audio data into memory.
 * non-NULL return is pointer to new struct WaveFile, completed.
 * if buffered, the audio data has already been allocated and filled,
 * and the file handle has been closed.
 * if not buffered, wf_FileHandle is ready to read audio data
 * if return NULL, all resources are already freed.
 */
struct WaveFile *loadWave(STRPTR fileName, BOOL buffered)
{
	struct WaveFile *wave = IExec->AllocVecTags(
		sizeof(struct WaveFile),
		AVT_ClearWithValue,0,
		TAG_END);
	if(!wave)
	{
		return(freeWave(wave, "Failed to allocate struct WaveFile\n"));
	}
	
	storeString(&wave->wf_FileName, fileName);

	if(!(wave->wf_FileHandle = IDOS->FOpen(wave->wf_FileName, MODE_OLDFILE, 0)))
	{
		return(freeWave(wave, "loadWave(%s, %s) failed to open file\n", 
			wave->wf_FileName, buffered?"TRUE":"FALSE"));
	}

	uint8 buffer[12], bytesRead;
	bytesRead = IDOS->FRead(wave->wf_FileHandle, buffer, 1, 12);
	if(bytesRead != 12)
	{
		return(freeWave(wave, "file %s is not big enough to be a Wave file\n", 
			wave->wf_FileName));
	}

	if(!((buffer[0]  == 'R')
		&&(buffer[1]  == 'I')
		&&(buffer[2]  == 'F')
		&&(buffer[3]  == 'F')
		&&(buffer[8]  == 'W')
		&&(buffer[9]  == 'A')
		&&(buffer[10] == 'V')
		&&(buffer[11] == 'E')))
	{
		return(freeWave(wave, "file %s is not a Wave file\n", 
			wave->wf_FileName));
	}

	if(!(seek4(wave->wf_FileHandle, 'f', 'm', 't', ' ')))
	{
		return(freeWave(wave, "could not find \"fmt \" chunk in file %s\n", 
			wave->wf_FileName));
	}
	wave->wf_FmtSize      = readLE(wave->wf_FileHandle, 4);
	if(wave->wf_FmtSize != 16)
	{
		IDOS->Printf("file %s wave format size %ld != 16\n",wave->wf_FileName, wave->wf_FmtSize);
	}
	wave->wf_AudioFormat  = readLE(wave->wf_FileHandle, 2);
	if(wave->wf_AudioFormat != 1)
	{
		IDOS->Printf("file %s AudioFormat %ld != 1\n",wave->wf_FileName, wave->wf_AudioFormat);
	}
	wave->wf_ChannelCount = readLE(wave->wf_FileHandle, 2);
	wave->wf_SampleRate   = readLE(wave->wf_FileHandle, 4);
	IDOS->ChangeFilePosition(wave->wf_FileHandle, 6LL, OFFSET_CURRENT);
	wave->wf_SampleWidth  = readLE(wave->wf_FileHandle, 2);

	if(!(seek4(wave->wf_FileHandle, 'd', 'a', 't', 'a')))
	{
		return(freeWave(wave, "could not find \"data\" chunk in file %s\n", 
				wave->wf_FileName));
	}
	wave->wf_DataSize   = readLE(wave->wf_FileHandle, 4);

	if(Verbose)
	{
		IDOS->Printf("loadWave(\"%s\", %s)\n", wave->wf_FileName, buffered?"TRUE":"FALSE");
		IDOS->Printf("\tWave Channel Count %ld\n", wave->wf_ChannelCount);
		IDOS->Printf("\tWave Sample Rate %ld\n", wave->wf_SampleRate);
		IDOS->Printf("\tWave Sample Width %ld\n", wave->wf_SampleWidth);
		IDOS->Printf("\tAudio Format %ld\n", wave->wf_AudioFormat);
		IDOS->Printf("\tData Size %ld\n", wave->wf_DataSize);
	}

	if(!buffered)
	{	// player will read the open wave->wh_FileHandle directly
		return(wave);
	}

	if(!(wave->wf_AudioData = IExec->AllocVecTags(wave->wf_DataSize)))
	{
		return(freeWave(wave, "loadWave(%s,%s) failed to allocate %ld bytes of audio data\n",
			wave->wf_FileName, buffered?"TRUE":"FALSE",wave->wf_DataSize));
	}

	bytesRead = IDOS->FRead(wave->wf_FileHandle, wave->wf_AudioData, 1, wave->wf_DataSize);

	if(bytesRead != wave->wf_DataSize)
	{
		IDOS->Printf("actual read is %ld\n", bytesRead);
		return(freeWave(wave, "loadWave(%s,%s) failed to read %ld bytes of audio data\n",
			wave->wf_FileName, buffered?"TRUE":"FALSE", wave->wf_DataSize));
	}

	IDOS->FClose(wave->wf_FileHandle);
	wave->wf_FileHandle = 0;

	return(wave);
}

/* after saving AudioData, fill out the WaveFile members
 * wf_AudioFormat (probably 1)
 * wf_ChannelCount, wf_SampleRate, wf_SampleWidth to correct values,
 * then call here to finalize and close the file
 */
BOOL finishWave(struct WaveFile *wave)
{
	if(!wave->wf_FileHandle)
	{
		return(FALSE);
	}
	int64 fileSize = IDOS->GetFileSize(
		wave->wf_FileHandle);
	writeLE32(wave->wf_FileHandle, WAVE_FileSize, fileSize-8);
	writeLE8( wave->wf_FileHandle, WAVE_Format, wave->wf_AudioFormat);
	writeLE16(wave->wf_FileHandle, WAVE_ChannelCount, wave->wf_ChannelCount);
	writeLE32(wave->wf_FileHandle, WAVE_SampleRate, wave->wf_SampleRate);
	writeLE32(wave->wf_FileHandle, WAVE_BytesPerSecond, 
		wave->wf_SampleRate * wave->wf_ChannelCount 
		 * byteWidth(wave->wf_SampleWidth) );
	writeLE16(wave->wf_FileHandle, WAVE_BytesPerPacket,
		wave->wf_ChannelCount * byteWidth(wave->wf_SampleWidth));
	writeLE16(wave->wf_FileHandle, WAVE_BitsPerSample,
		byteWidth(wave->wf_SampleWidth) * 8);
	writeLE32(wave->wf_FileHandle, WAVE_DataSize, fileSize-44);

	IDOS->FClose(wave->wf_FileHandle);
	wave->wf_FileHandle = 0;
	return(TRUE);
}

static void writeLE32(BPTR file, 
	int32 offset, uint32 value)
{
	if(!file)
	{
		return;
	}
	IDOS->ChangeFilePosition(
		file,
		offset,
		OFFSET_BEGINNING);
	IDOS->FPutC(file, (value >>  0 & 0xFF));
	IDOS->FPutC(file, (value >>  8 & 0xFF));
	IDOS->FPutC(file, (value >> 16 & 0xFF));
	IDOS->FPutC(file, (value >> 24 & 0xFF));
}

static void writeLE16(BPTR file, 
	int32 offset, uint16 value)
{
	if(!file)
	{
		return;
	}
	IDOS->ChangeFilePosition(
		file,
		offset,
		OFFSET_BEGINNING);
	IDOS->FPutC(file, (value & 0x00FF));
	IDOS->FPutC(file, (value & 0xFF00)>>8);
}

static void writeLE8(BPTR file, 
 	int32 offset, uint8 value)
{
	if(!file)
	{
		return;
	}
	IDOS->ChangeFilePosition(
		file,
		offset,
		OFFSET_BEGINNING);
	IDOS->FPutC(file, value);
}

// corrects 24 bits to 32, actual container width
uint32 byteWidth(uint32 sampleWidth)
{
	if(sampleWidth <= 8)
	{
		return(1);
	}
	if(sampleWidth <= 16)
	{
		return(2);
	}
	return(4);
}	

// read nBytes from file, with endian swap
// returns value, or -1 for EOF
int32 readLE(BPTR file, uint32 nBytes)
{
	uint32 mult = 1, result = 0;
	int32 next;

	while(nBytes--)
	{
		next = IDOS->FGetC(file);

		if(next < 0)
		{
			return(next);
		}
		result += next * mult;
		mult *= 256;
	}
	return(result);
}

// seek a 4 byte sequence from current position in the open file.
// returns TRUE with file position immediately after if found
// returns FALSE with file position where it was if not found
// HORRIBLY inefficient, but simple.
BOOL seek4(BPTR file, uint32 c0, uint32 c1, uint32 c2, uint32 c3)
{
	uint64 start = IDOS->GetFilePosition(file);
	int32 next;

	IDOS->ChangeFilePosition(file, 0LL, OFFSET_BEGINNING);
	while(-1 != (next=IDOS->FGetC(file)))
	{
		if(c0 == next)
		{
			next = IDOS->FGetC(file);
			if(c1 == next)
			{
				next = IDOS->FGetC(file);
				if(c2 == next)
				{
					next = IDOS->FGetC(file);
					if(c3 == next)
					{
						return(TRUE);
					}
					else
					{
						IDOS->ChangeFilePosition(file, -1LL, OFFSET_CURRENT);
					}
				}
				else
				{
					IDOS->ChangeFilePosition(file, -1LL, OFFSET_CURRENT);
				}
			}
			else
			{
				IDOS->ChangeFilePosition(file, -1LL, OFFSET_CURRENT);
			}
		}
	}

	IDOS->ChangeFilePosition(file, start, OFFSET_BEGINNING);
	return(FALSE);	
}


