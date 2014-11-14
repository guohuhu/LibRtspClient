// libRtspAccess.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include <stdio.h>

#include "libRtspAccess.h"
#include "LibRtsp.h"
#include "VideoDecode.h"


CLibRtsp gLibRtsp;

int WINAPI RTSP_StartStream(const char* pRtspUrl,void* pUserData, STREAM_OVER_PROTOCOL Stream_Over_Protocol)
{
 	char tempstr[256] = {0};
 	sprintf(tempstr, "\n------------------------------------------------------------------------\nRTSP_StartStream url: %s\n------------------------------------------------------------------------\n", pRtspUrl);
 	OutputDebugStringA(tempstr);
	return gLibRtsp.StartStream(pRtspUrl, pUserData, Stream_Over_Protocol);

}

void WINAPI RTSP_SetCallBack(YUVDataCallBackFuction avDataCallBack,int id)
{
	gLibRtsp.SetAvDataCallBack(avDataCallBack,id);	
}
//int WINAPI RtspStartStream(const char* pRtspUrl,int image_width,int image_height,void* pUserData,STREAM_OVER_PROTOCOL Stream_Over_Protocol)
//{
//	return gLibRtsp.StartStream(pRtspUrl,NULL,pUserData,Stream_Over_Protocol);
//}

int WINAPI RTSP_StopStream(int id)
{
//	OutputDebugStringA("\n------------------\nRTSP_StopStream\n------------------\n");
	return gLibRtsp.StopStream(id);
}

int WINAPI RTSP_CloseAllStream()
{
	return gLibRtsp.CloseAllStream();
}

int WINAPI GetImage(unsigned char *imgPtr)
{
	return gLibRtsp.GetBmpImage(imgPtr);
}

int WINAPI FreeImage()
{
	return gLibRtsp.FreeImageBuffer();
}

