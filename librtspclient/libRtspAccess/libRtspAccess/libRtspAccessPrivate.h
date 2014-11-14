#ifndef RTSP_ACCESS_PRIVATE_INCLUDE
#define RTSP_ACCESS_PRIVATE_INCLUDE

#ifdef _DEBUG
#define OUTPUT_DEBUG_STRING(str,...) \
	do{	char tmpstr[1024] = {0}; \
	char tmpbuf[1024] = {0}; \
	char tmpdt[1024] = {0}; \
	time_t t; \
	char* dt = NULL; \
	t=time(NULL); \
	dt = ctime(&t); \
	strncat_s(tmpdt, dt, strlen(dt) - 1); \
	sprintf_s(tmpstr,sizeof(tmpbuf),str,__VA_ARGS__) ; \
	sprintf_s(tmpbuf, "[ libRtsp %s ]: %s", tmpdt, tmpstr); \
	OutputDebugStringA(tmpbuf);}while(0)
#else
#define OUTPUT_DEBUG_STRING(str,...);
#endif

typedef void (CALLBACK *LostDetectCallBackFuction)(time_t LastTime);

typedef int (CALLBACK *AVDataCallBackFuction)(int iAVSelect, char* pAVData, int iDataLen, int iWidth, int iHeight, int SessionID, void* pUserData);

#endif