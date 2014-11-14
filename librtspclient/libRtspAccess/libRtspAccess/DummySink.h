#pragma once

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "mediasink.hh"

#include "RtspImpletement.h"
#include "libRtspAccessPrivate.h"
#include "libRtspAccess.h"



class DummySink :
	public MediaSink
{
public:
	static DummySink* createNew(UsageEnvironment& env,
		MediaSubsession& subsession, // identifies the kind of data that's being received
		char const* streamId = NULL); // identifies the stream itself (optional)
	
	int SetCallBackFunction(AVDataCallBackFuction AVDataFun, LostDetectCallBackFuction LostDetectFun, int SessionID, void* pUserData);

	void SetLastRecvTime(time_t time);
	time_t GetLastRecvTime();
private:
	DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);
	// called only by "createNew()"
	virtual ~DummySink();

	static void afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
	void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);

private:
	// redefined virtual functions:
	virtual Boolean continuePlaying();
	char* m_avdata;
	void* m_UserData;

private:
	u_int8_t* fReceiveBuffer;
	MediaSubsession& fSubsession;
	char* fStreamId;
	
	int m_width;
	int m_height;

	
};
