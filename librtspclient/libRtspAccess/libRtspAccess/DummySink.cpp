#include "StdAfx.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "DummySink.h"

#include "h264_stream.h"

AVDataCallBackFuction m_AVDataFun;
LostDetectCallBackFuction m_LostDetectFun;





// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 512*1024

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId) {
	return new DummySink(env, subsession, streamId);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
: MediaSink(env),
fSubsession(subsession) {
	fStreamId = strDup(streamId);
	fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];

	m_height = m_width = 0;

	m_avdata = new char[512*1024];
	m_UserData = NULL;
}

DummySink::~DummySink() {


 	delete[] fReceiveBuffer;
	delete[] fStreamId;
	delete[] m_avdata;
}

void DummySink::SetLastRecvTime(time_t time)
{

}
time_t DummySink::GetLastRecvTime()
{
	time_t temp = 0;

	
	return temp;
}


void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
struct timeval presentationTime, unsigned durationInMicroseconds) {
	DummySink* sink = (DummySink*)clientData;

// 	char temp[64] = {0};
// 	sprintf(temp, "%s %d %p\n", __FUNCTION__, __LINE__, clientData);
// 	OutputDebugStringA(temp);

	sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

int DummySink::SetCallBackFunction(AVDataCallBackFuction AVDataFun, LostDetectCallBackFuction LostDetectFun, int SessionID, void* pUserData)
{

	m_LostDetectFun = LostDetectFun;
	m_AVDataFun = AVDataFun;
	m_UserData = pUserData;
	return 0;
}


// If you don't want to see debugging output for each received frame, then comment out the following line:
//#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1

void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
	// We've just received a frame of data.  (Optionally) print out information about it:
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
	if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
	envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
	if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes trun cated)";
	char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
	sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
	envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
	if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
		envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
	}
#ifdef DEBUG_PRINT_NPT
	envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
	envir() << "\n";
#endif

#if 1
	// Then continue, to request the next frame of data:
	if (m_LostDetectFun != NULL)
	{
		m_LostDetectFun(time(NULL));
	}
	
	//unsigned char *nalPtr = fReceiveBuffer;
//	if(nalPtr[0] == 0x00 && nalPtr[1] == 0x00 && nalPtr[2] == 0x01){
//		nalPtr = nalPtr + 3;
//	}

#if 1
	OutputDebugStringA("DummySink::afterGettingFrame\n");
	OUTPUT_DEBUG_STRING("stream url is %s",fStreamId);
	//char *mRtspUrl = new char[strlen(fStreamId)];
	//memset(mRtspUrl,0,sizeof(mRtspUrl));
	//memcpy(mRtspUrl,fStreamId,strlen(fStreamId));
	if(strcmp(fSubsession.mediumName(), "video") == 0)
	{
		unsigned char nal = fReceiveBuffer[0] & 0x1f;
		if ( nal == 0x07)
		{

			h264_stream_t h = {0};
			read_nal_unit(&h, (unsigned char*)(fReceiveBuffer), frameSize);


 			OUTPUT_DEBUG_STRING("w = %d, h = %d pThis = %p\n", (h.sps.pic_width_in_mbs_minus1 + 1) * 16, (h.sps.pic_height_in_map_units_minus1 + 1) * 16, this);
			OUTPUT_DEBUG_STRING("num = %d\n", h.sps.num_ref_frames);
			
			m_height = (h.sps.pic_height_in_map_units_minus1 + 1) * 16 * h.sps.num_ref_frames;
			m_width = (h.sps.pic_width_in_mbs_minus1 + 1) * 16;

		}
	}
#endif
#if 1
	if (m_AVDataFun != NULL)
	{

		if(strcmp(fSubsession.mediumName(), "video") == 0)
		{
			char head[4] = { 0x00, 0x00, 0x00, 0x01 };
			//char* tempdata = new char[frameSize + sizeof(head)];

			if (m_avdata != NULL && frameSize + sizeof(head) <= 512*1024)
			{
				memcpy(m_avdata, head, sizeof(head));
				memcpy(m_avdata + sizeof(head), fReceiveBuffer, frameSize);
				
				
				m_AVDataFun(0, m_avdata, sizeof(head) + frameSize, m_width, m_height, 0,fStreamId);

			}

	
		}
		else if(strcmp(fSubsession.mediumName(), "audio") == 0)
		{
			m_AVDataFun(1, (char *)fReceiveBuffer, frameSize, 0, 0, 0, m_UserData);
		}

	}
#endif
	//delete mRtspUrl;
	//ProcFrameData(frameSize);
#endif
	
	continuePlaying();
}

Boolean DummySink::continuePlaying() {
	if (fSource == NULL) return False; // sanity check (should not happen)

	// Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
	fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
		afterGettingFrame, this,
		onSourceClosure, this);
	return True;
}




