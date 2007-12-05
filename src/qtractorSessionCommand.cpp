// qtractorSessionCommand.cpp
//
/****************************************************************************
   Copyright (C) 2005-2007, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "qtractorAbout.h"
#include "qtractorSessionCommand.h"
#include "qtractorClipCommand.h"

#include "qtractorMidiEngine.h"
#include "qtractorAudioClip.h"


//----------------------------------------------------------------------
// class qtractorSessionCommand - implementation
//

// Constructor.
qtractorSessionCommand::qtractorSessionCommand ( const QString& sName,
	qtractorSession *pSession ) : qtractorCommand(sName), m_pSession(pSession)
{
}


// Destructor.
qtractorSessionCommand::~qtractorSessionCommand (void)
{
}


//----------------------------------------------------------------------
// class qtractorSessionTempoCommand - implementation
//

// Constructor.
qtractorSessionTempoCommand::qtractorSessionTempoCommand (
	qtractorSession *pSession, float fTempo )
	: qtractorSessionCommand(QObject::tr("session tempo"), pSession)
{
	m_fTempo = fTempo;

	// Take care of time-stretching of all audio-clips...
	m_pClipCommand = NULL;
	for (qtractorTrack *pTrack = pSession->tracks().first();
			pTrack; pTrack = pTrack->next()) {
		if (pTrack->trackType() == qtractorTrack::Audio) {
			for (qtractorClip *pClip = pTrack->clips().first();
					pClip; pClip = pClip->next()) {
				qtractorAudioClip *pAudioClip
					= static_cast<qtractorAudioClip *> (pClip);
				if (pAudioClip) {
					if (m_pClipCommand == NULL)
						m_pClipCommand = new qtractorClipCommand(name());
					float fTimeStretch
						= (fTempo * pAudioClip->timeStretch())
							/ pSession->tempo();
					m_pClipCommand->timeStretchClip(pClip, fTimeStretch);
				}
			}
		}
	}		
}

// Desstructor.
qtractorSessionTempoCommand::~qtractorSessionTempoCommand (void)
{
	if (m_pClipCommand)
		delete m_pClipCommand;
}


// Session-tempo command methods.
bool qtractorSessionTempoCommand::redo (void)
{
	qtractorSession *pSession = session();
	if (pSession == NULL)
		return false;

	// Save the previous session tempo alright...
	float fTempo = pSession->tempo();

	// If currently playing, we need to do a stop and go...
	bool bPlaying = pSession->isPlaying();
	if (bPlaying)
		pSession->lock();

	// Just set new one...
	pSession->setTempo(m_fTempo);

	// In case we have audio clips around...
	if (m_pClipCommand)
		m_pClipCommand->redo();

	// Restore playback state, if needed...
	if (bPlaying) {
		// The MIDI engine queue needs a reset...
		if (pSession->midiEngine())
			pSession->midiEngine()->resetTempo();
		pSession->unlock();
	}

	// Swap it nice, finally.
	m_fTempo = fTempo;

	return true;
}

bool qtractorSessionTempoCommand::undo (void)
{
	// As we swap the prev/tempo this is non-idempotent.
	return redo();
}


//----------------------------------------------------------------------
// class qtractorSessionLoopCommand - implementation
//

// Constructor.
qtractorSessionLoopCommand::qtractorSessionLoopCommand (
	qtractorSession *pSession, unsigned long iLoopStart, unsigned long iLoopEnd )
	: qtractorSessionCommand(QObject::tr("session loop"), pSession)
{
	m_iLoopStart = iLoopStart;
	m_iLoopEnd   = iLoopEnd;
}


// Session-loop command methods.
bool qtractorSessionLoopCommand::redo (void)
{
	qtractorSession *pSession = session();
	if (pSession == NULL)
		return false;

	// Save the previous session loop state alright...
	unsigned long iLoopStart = pSession->loopStart();
	unsigned long iLoopEnd   = pSession->loopEnd();

	// Just set new bounds...
	pSession->setLoop(m_iLoopStart, m_iLoopEnd);

	// Restore edit cursors too...
	if (m_iLoopStart < m_iLoopEnd) {
		pSession->setEditHead(m_iLoopStart);
		pSession->setEditTail(m_iLoopEnd);
	}

	// Swap it nice, finally.
	m_iLoopStart = iLoopStart;
	m_iLoopEnd   = iLoopEnd;

	return true;
}

bool qtractorSessionLoopCommand::undo (void)
{
	// As we swap the prev/tempo this is non-idempotent.
	return redo();
}


// end of qtractorSessionCommand.cpp
