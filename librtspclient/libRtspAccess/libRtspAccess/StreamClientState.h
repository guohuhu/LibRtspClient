#pragma once


#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

class StreamClientState
{
public:
	StreamClientState(void);
	virtual ~StreamClientState(void);

	MediaSubsessionIterator* iter;
	MediaSession* session;
	MediaSubsession* subsession;
	TaskToken streamTimerTask;
	double duration;
};
