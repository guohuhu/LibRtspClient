#ifndef RTSP_ACCESS_INCLUDE
#define RTSP_ACCESS_INCLUDE

#define RTSP_API extern "C"__declspec( dllexport )

typedef enum 
{
	STREAMING_OVER_UDP = 0,
	STREAMING_OVER_TCP
} STREAM_OVER_PROTOCOL;


//typedef int (CALLBACK *AVDataCallBackFuction)(int iAVSelect, char* pAVData, int iDataLen, int iWidth, int iHeight, int SessionID, void* pUserData);
typedef int (CALLBACK *_YUVDataCallBackFuction)(char* pAVData, int iDataLen, int iWidth, int iHeight);

RTSP_API int WINAPI RTSP_StartStream(const char* pRtspUrl,  
								void* pUserData, 
								STREAM_OVER_PROTOCOL Stream_Over_Protocol = STREAMING_OVER_UDP);
RTSP_API void WINAPI RTSP_SetCallBack(_YUVDataCallBackFuction avDataCallBack,int id);

RTSP_API int WINAPI RTSP_StopStream(int id);
RTSP_API int WINAPI RTSP_CloseAllStream();

//RTSP_API int WINAPI RtspStartStream(const char* pRtspUrl,int image_width,int image_height,void* pUserData,STREAM_OVER_PROTOCOL Stream_Over_Protocol = STREAMING_OVER_UDP);

RTSP_API int WINAPI GetImage(unsigned char *imgPtr);
RTSP_API int WINAPI FreeImage();
int CALLBACK AVDataCallBack(int iAVSelect, char* pAVData, int iDataLen, int iWidth, int iHeight, int SessionID, void* pUserData);

#endif


