
#include "stdafx.h"
/*#include "vld.h"*/
#include <time.h>

#include "RtspImpletement.h"
#include "libRtspAccessPrivate.h"

#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"

#pragma comment (lib, "Ws2_32.lib") 
#pragma comment (lib, "BasicUsageEnvironment.lib")  
#pragma comment (lib, "groupsock.lib")  
#pragma comment (lib, "liveMedia.lib")  
#pragma comment (lib, "UsageEnvironment.lib")  

RTSPClient* gRtspClient = NULL;
TaskScheduler* gScheduler = NULL;
UsageEnvironment* gEnv = NULL;

AVDataCallBackFuction gAVDataFun = NULL;
void* gUserData = NULL;
LostDetectCallBackFuction gLostDetectFun = NULL;

STREAM_OVER_PROTOCOL gStream_Over_Protocol = STREAMING_OVER_UDP;

char eventLoopWatchVariable = 0;
char chRtspUrl[256] = {0};

static int sum_add = 0;
static int sum_del = 0;

static int close_times = 0;
static int open_times = 0;


// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
	return env;// << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
	return env;// << subsession.mediumName() << "/" << subsession.codecName();
}


#if 0
void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL, LostDetectCallBackFuction func) {
	// Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
	// to receive (even if more than stream uses the same "rtsp://" URL).
	RTSPClient* rtspClient = ourRTSPClient::createNew(env, rtspURL);//, RTSP_CLIENT_VERBOSITY_LEVEL, progName);
	if (rtspClient == NULL) {
		//env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
		OUTPUT_DEBUG_STRING("Failed to create a RTSP client for URL %s\": %s\n", rtspURL, env.getResultMsg());
		return;
	}
	
	OUTPUT_DEBUG_STRING("open times = %d close times = %d\n", ++open_times, close_times);

	gRtspClient = rtspClient;
	gLostDetectFun = func;
	// Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
	// Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
	// Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
	rtspClient->sendDescribeCommand(continueAfterDESCRIBE); 
}
#endif

void *openURL(UsageEnvironment& env, char const* progName, char const* rtspURL,LostDetectCallBackFuction func) {
	// Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
	// to receive (even if more than stream uses the same "rtsp://" URL).
	//std::string RtspUrlStr(rtspURL);
	//RtspUrlStr.find(gParamVector.at(i).RtspUrl) == 0
	RTSPClient* rtspClient = ourRTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName);
	
	//gRtspClientVector.insert(rtspClient);
	if (rtspClient == NULL) {
		env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
		return NULL;
	}
	//++rtspClientCount;
	gRtspClient = rtspClient;
	gLostDetectFun = func;

	// Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
	// Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
	// Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
	rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
	
	
	return rtspClient;
}


// Implementation of the RTSP 'response handlers':

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
	OUTPUT_DEBUG_STRING("%s \n", __FUNCTION__);
	do {
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

		if (resultCode != 0) {
			//env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
			OUTPUT_DEBUG_STRING("Failed to get a SDP description: %s\n", resultString);
			delete[] resultString;
			break;
		}

		char* const sdpDescription = resultString;
		//env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";
		//OUTPUT_DEBUG_STRING("Got a SDP description:\n %s\n", sdpDescription);

		// Create a media session object from this SDP description:
		scs.session = MediaSession::createNew(env, sdpDescription);
		delete[] sdpDescription; // because we don't need it anymore
		if (scs.session == NULL) {
			//env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
			OUTPUT_DEBUG_STRING("Failed to create a MediaSession object from the SDP description: %s\n", env.getResultMsg());
			break;
		} else if (!scs.session->hasSubsessions()) {
			//env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
			OUTPUT_DEBUG_STRING("This session has no media subsessions (i.e., no \"m=\" lines)\n");
			break;
		}

		// Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
		// calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
		// (Each 'subsession' will have its own data source.)
		scs.iter = new MediaSubsessionIterator(*scs.session);
		setupNextSubsession(rtspClient);
		return;
	} while (0);

	// An unrecoverable error occurred with this stream.
	shutdownStream(rtspClient);
}

// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP False

void setupNextSubsession(RTSPClient* rtspClient) {

	OUTPUT_DEBUG_STRING("%s \n", __FUNCTION__);
	UsageEnvironment& env = rtspClient->envir(); // alias
	StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

	scs.subsession = scs.iter->next();
	if (scs.subsession != NULL) {
		if (!scs.subsession->initiate()) {
			//env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
			OUTPUT_DEBUG_STRING("Failed to initiate the \" %p \" subsession: %s \n", *scs.subsession, env.getResultMsg());
			setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
		} else {
			OUTPUT_DEBUG_STRING("Initiated the subsession %p (client ports %u-%u)\n", \
				scs.subsession, \
				scs.subsession->clientPortNum(), \
				scs.subsession->clientPortNum() + 1);

			//env << *rtspClient << "Initiated the \"" << *scs.subsession << "\" subsession (client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1 << ")\n";

			// Continue setting up this subsession, by sending a RTSP "SETUP" command:

			Boolean stream_over_tcp = False;

			if (gStream_Over_Protocol == STREAMING_OVER_TCP)
			{
				stream_over_tcp = True;
			}
			else if (gStream_Over_Protocol == STREAMING_OVER_UDP)
			{
				stream_over_tcp = False;
			}
			
			
			rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, stream_over_tcp);
		}
		return;
	}

	// We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
	if (scs.session->absStartTime() != NULL) {
		// Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
		rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime());
	} else {
		scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
		rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
	}
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
	OUTPUT_DEBUG_STRING("%s \n", __FUNCTION__);
	do {
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

		static int SessionID = 0;

		if (resultCode != 0) {
			OUTPUT_DEBUG_STRING("Failed to set up the subsession: %s \n", \
				resultString);
			//env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
			break;
		}

		OUTPUT_DEBUG_STRING("Set up the subsession (client ports %u-%u)\n", \
			scs.subsession->clientPortNum(), scs.subsession->clientPortNum()+1);
		//env << *rtspClient << "Set up the \"" << *scs.subsession << "\" subsession (client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1 << ")\n";

		// Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
		// (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
		// after we've sent a RTSP "PLAY" command.)

		scs.subsession->sink = DummySink::createNew(env, *scs.subsession, rtspClient->url());

		OUTPUT_DEBUG_STRING("add = %d, del = %d\n", ++sum_add, sum_del);

		((DummySink *)scs.subsession)->SetCallBackFunction(gAVDataFun, gLostDetectFun, SessionID, rtspClient);
		SessionID ++;
		// perhaps use your own custom "MediaSink" subclass instead
		if (scs.subsession->sink == NULL) {
			OUTPUT_DEBUG_STRING("Failed to create a data sink for the subsession: %s\n", \
				env.getResultMsg());
			//env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession<< "\" subsession: " << env.getResultMsg() << "\n";
			break;
		}
		OUTPUT_DEBUG_STRING("Created a data sink for the subsession\n");
		//env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
		scs.subsession->miscPtr = rtspClient; // a hack to let subsession handle functions get the "RTSPClient" from the subsession 
		scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
			subsessionAfterPlaying, scs.subsession);
		// Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
		if (scs.subsession->rtcpInstance() != NULL) {
			scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
		}
	} while (0);
	delete[] resultString;

	// Set up the next subsession, if any:
	setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
	Boolean success = False;
	OUTPUT_DEBUG_STRING("%s \n", __FUNCTION__);

	do {
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

		if (resultCode != 0) {
			//env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
			OUTPUT_DEBUG_STRING("Failed to start playing session: %s \n", resultString);
			break;
		}

		// Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
		// using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
		// 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
		// (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
		if (scs.duration > 0) {
			unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
			scs.duration += delaySlop;
			unsigned uSecsToDelay = (unsigned)(scs.duration*1000000);
			scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
		}

		//env << *rtspClient << "Started playing session";
		OUTPUT_DEBUG_STRING("Started playing session");
		if (scs.duration > 0) {
			//env << " (for up to " << scs.duration << " seconds)";
			OUTPUT_DEBUG_STRING("(for up to %d seconds)", scs.duration);
		}
		//env << "...\n";
		OUTPUT_DEBUG_STRING("...\n");

		success = True;
	} while (0);
	delete[] resultString;

	if (!success) {
		// An unrecoverable error occurred with this stream.
		shutdownStream(rtspClient);
	}
}


// Implementation of the other event handlers:

void subsessionAfterPlaying(void* clientData) {
	OUTPUT_DEBUG_STRING("%s \n", __FUNCTION__);
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

	// Begin by closing this subsession's stream:
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	// Next, check whether *all* subsessions' streams have now been closed:
	MediaSession& session = subsession->parentSession();
	MediaSubsessionIterator iter(session);
	while ((subsession = iter.next()) != NULL) {
		if (subsession->sink != NULL) return; // this subsession is still active
	}

	// All subsessions' streams have now been closed, so shutdown the client:
	shutdownStream(rtspClient);
}

void subsessionByeHandler(void* clientData) {
	OUTPUT_DEBUG_STRING("%s \n", __FUNCTION__);
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
	UsageEnvironment& env = rtspClient->envir(); // alias

	//env << *rtspClient << "Received RTCP \"BYE\" on \"" << *subsession << "\" subsession\n";
	OUTPUT_DEBUG_STRING("Received RTCP \"BYE\" on subsession\n");
	// Now act as if the subsession had closed:
	subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void* clientData) {
	OUTPUT_DEBUG_STRING("%s \n", __FUNCTION__);
	ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
	StreamClientState& scs = rtspClient->scs; // alias

	scs.streamTimerTask = NULL;

	// Shut down the stream:
	shutdownStream(rtspClient);
}
#if 0
void shutdownStream(RTSPClient* rtspClient, int exitCode) {
	OUTPUT_DEBUG_STRING("%s \n", __FUNCTION__);
	
	OUTPUT_DEBUG_STRING("open times = %d close times = %d\n", open_times, ++close_times);


	UsageEnvironment& env = rtspClient->envir(); // alias
	StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

	// First, check whether any subsessions have still to be closed:
	if (scs.session != NULL) { 
		Boolean someSubsessionsWereActive = False;
		MediaSubsessionIterator iter(*scs.session);
		MediaSubsession* subsession;

		while ((subsession = iter.next()) != NULL) {
			if (subsession->sink != NULL) {
				//subsession->sink->stopPlaying();
				Medium::close(subsession->sink);
				subsession->sink = NULL;

				OUTPUT_DEBUG_STRING("22 add = %d, del = %d\n", sum_add, ++sum_del);
				
				if (subsession->rtcpInstance() != NULL) {
					subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
				}
				someSubsessionsWereActive = True;
			}
		}

		if (someSubsessionsWereActive) {
			// Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
			// Don't bother handling the response to the "TEARDOWN".
			//gEnv->taskScheduler().unscheduleDelayedTask(scs.streamTimerTask);
			rtspClient->sendTeardownCommand(*scs.session, NULL);
			
		}
		
		//Medium::close(scs.session);
		//gEnv->taskScheduler().unscheduleDelayedTask(scs.streamTimerTask);
		Medium::close(rtspClient);
		rtspClient = NULL;
		
		//gEnv->reclaim(); //del by huguohu
		//gEnv = NULL;
		if(gScheduler != NULL){
			delete gScheduler; 
			gScheduler = NULL;	
		}

	}

	

	

	


	//env << *rtspClient << "Closing the stream.\n";
	//Medium::close(scs.session);
	

	// Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.


	// 
	// 	if (--rtspClientCount == 0) {
	// 		// The final stream has ended, so exit the application now.
	// 		// (Of course, if you're embedding this code into your own application, you might want to comment this out,
	// 		// and replace it with "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop, and continue running "main()".)
	// 		//exit(exitCode);
	// 	}

	
}
#endif

void shutdownStream(RTSPClient* rtspClient, int exitCode) {
	UsageEnvironment& env = rtspClient->envir(); // alias
	StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

	// First, check whether any subsessions have still to be closed:
	if (scs.session != NULL) { 
		Boolean someSubsessionsWereActive = False;
		MediaSubsessionIterator iter(*scs.session);
		MediaSubsession* subsession;

		while ((subsession = iter.next()) != NULL) {
			if (subsession->sink != NULL) {
				Medium::close(subsession->sink);
				subsession->sink = NULL;

				if (subsession->rtcpInstance() != NULL) {
					subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
				}

				someSubsessionsWereActive = True;
			}
		}

		if (someSubsessionsWereActive) {
			// Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
			// Don't bother handling the response to the "TEARDOWN".
			rtspClient->sendTeardownCommand(*scs.session, NULL);
		}
	}

	env << *rtspClient << "Closing the stream.\n";
	Medium::close(rtspClient);
	// Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.

//	if (--rtspClientCount == 0) {
		// The final stream has ended, so exit the application now.
		// (Of course, if you're embedding this code into your own application, you might want to comment this out,
		// and replace it with "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop, and continue running "main()".)
//		exit(exitCode);
//	}
}