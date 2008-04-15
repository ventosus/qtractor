// qtractorAudioBuffer.h
//
/****************************************************************************
   Copyright (C) 2005-2008, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#ifndef __qtractorAudioBuffer_h
#define __qtractorAudioBuffer_h

#include "qtractorList.h"
#include "qtractorAudioFile.h"
#include "qtractorRingBuffer.h"

#ifdef CONFIG_LIBSAMPLERATE
// libsamplerate API
#include <samplerate.h>
#endif

#include <QThread>
#include <QMutex>
#include <QWaitCondition>


// Forward declarations.
class qtractorAudioBufferThread;
class qtractorTimeStretcher;


//----------------------------------------------------------------------
// class qtractorAudioBuffer -- Ring buffer/cache template declaration.
//

class qtractorAudioBuffer : public qtractorList<qtractorAudioBuffer>::Link
{
public:

	// Constructors.
	qtractorAudioBuffer(unsigned short iChannels, unsigned int iSampleRate);
	// Default destructor.
	~qtractorAudioBuffer();

	// Internal file descriptor accessors.
	qtractorAudioFile* file() const;

	// File implementation properties.
	unsigned short channels() const;
	unsigned long frames() const;
	unsigned int sampleRate() const;

	// Resample ratio accessor.
	float resampleRatio() const;

	// Operational initializer/terminator.
	bool open(const QString& sFilename, int iMode = qtractorAudioFile::Read);
	void close();

	// Buffer data read/write.
	int read(float **ppFrames, unsigned int iFrames,
		unsigned int iOffset = 0);
	int write(float **ppFrames, unsigned int iFrames,
		unsigned short iChannels = 0);

	// Special kind of super-read/channel-mix.
	int readMix(float **ppFrames, unsigned int iFrames,
		unsigned short iChannels, unsigned int iOffset, float fGain);

	// Buffer data seek.
	bool seek(unsigned long iFrame);

	// Reset this buffer's state.
	void reset(bool bLooping);

	// Logical clip-offset (in frames from beginning-of-file).
	void setOffset(unsigned long iOffset);
	unsigned long offset() const;

	// Logical clip-length (in frames from clip-start/offset).
	void setLength(unsigned long iLength);
	unsigned long length() const;

	// Current (last known) file length accessor.
	unsigned long fileLength() const;

	// Loop points accessors.
	void setLoop(unsigned long iLoopStart, unsigned long iLoopEnd);
	unsigned long loopStart() const;
	unsigned long loopEnd() const;

	// Time-stretch factor.
	void setTimeStretch(float fTimeStretch);
	float timeStretch() const;
	bool isTimeStretch() const;

	// Pitch-shift factor.
	void setPitchShift(float fPitchShift);
	float pitchShift() const;
	bool isPitchShift() const;

	// Initial thread-sync executive (if file is on read mode,
	// check whether it can be cache-loaded integrally).
	bool initSync();

	// Base sync method.
	void sync();

	// Audio frame process synchronization predicate method.
	bool inSync(unsigned long iFrameStart, unsigned long iFrameEnd);

	// Export-mode sync executive.
	void syncExport();

	// Sample-rate converter type accessor (global option).
	static void setResampleType(int iResampleType);
	static int resampleType();

	// WSOLA time-stretch modes (global options).
	static void setWsolaTimeStretch(bool bWsolaTimeStretch);
	static bool isWsolaTimeStretch();

	static void setWsolaQuickSeek(bool bWsolaQuickSeek);
	static bool isWsolaQuickSeek();

#ifdef DEBUG
	void dump_state(const QString& sPrefix) const;
#endif

protected:

	// Read-sync mode methods (playback).
	void readSync();
	void readSyncIn();

	// Write-sync mode method (recording).
	void writeSync();

	// Internal-seek sync executive.
	bool seekSync(unsigned long iFrame);

	// Last-mile frame buffer-helper processor.
	int writeFrames(float **ppFrames, unsigned int iFrames);
	int flushFrames(unsigned int iFrames);

	// Buffer process methods.
	int readBuffer  (unsigned int iFrames);
	int writeBuffer (unsigned int iFrames);

	// Special kind of super-read/channel-mix buffer helper.
	int readMixFrames(float **ppFrames, unsigned int iFrames,
		unsigned short iChannels, unsigned int iOffset, float fGain);

	// I/O buffer release.
	void deleteIOBuffers();

	// Frame position converters.
	unsigned long framesIn(unsigned long iFrames) const;
	unsigned long framesOut(unsigned long iFrames) const;

private:

	// Audio file instance variables.
	unsigned short m_iChannels;
	unsigned int   m_iSampleRate;

	qtractorAudioFile *m_pFile;

	qtractorRingBuffer<float> *m_pRingBuffer;
	qtractorAudioBufferThread *m_pSyncThread;

	unsigned int   m_iThreshold;
	unsigned int   m_iBufferSize;
	bool           m_bReadSync;

	volatile unsigned long  m_iReadOffset;
	volatile unsigned long  m_iWriteOffset;

	unsigned long  m_iFileLength;
	bool           m_bIntegral;

	unsigned long  m_iOffset;
	unsigned long  m_iLength;

	unsigned long  m_iLoopStart;
	unsigned long  m_iLoopEnd;

	unsigned long  m_iSeekOffset;
	qtractorAtomic m_seekPending;

	float        **m_ppFrames;

	bool           m_bTimeStretch;
	float          m_fTimeStretch;

	bool           m_bPitchShift;
	float          m_fPitchShift;

	qtractorTimeStretcher *m_pTimeStretcher;

	float          m_fPrevGain;
	float          m_fNextGain;

#ifdef CONFIG_LIBSAMPLERATE
	bool           m_bResample;
	float          m_fResampleRatio;
	unsigned int   m_iInputPending;
	float        **m_ppInBuffer;
	float        **m_ppOutBuffer;
	SRC_STATE    **m_ppSrcState;
#endif

	// Sample-rate converter type global option.
	static int     g_iResampleType;

	// Time-stretch mode global options.
	static bool    g_bWsolaTimeStretch;
	static bool    g_bWsolaQuickSeek;
};


#endif  // __qtractorAudioBuffer_h


// end of qtractorAudioBuffer.h
