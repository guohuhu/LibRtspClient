// LibRtspTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include "libRtspAccess.h"
#include "libRtspAccessPrivate.h"
#include<cstdio>
#include<ctime>





int CALLBACK AVDataCallBackFun1(char* pAVData, int iDataLen, int iWidth, int iHeight)
{
	OUTPUT_DEBUG_STRING("AVDataCallBackFun1 Get YUV frame data Len is %d, width is %d, height is %d\n",iDataLen,iWidth,iHeight);
	return 0;
}

int CALLBACK AVDataCallBackFun2(char* pAVData, int iDataLen, int iWidth, int iHeight)
{
	OUTPUT_DEBUG_STRING("AVDataCallBackFun2 Get YUV frame data Len is %d, width is %d, height is %d\n",iDataLen,iWidth,iHeight);
	return 0;
}

//DWORD WINAPI OpenStream(LPVOID pParam)
//{
	//RtspStartStream("rtsp://192.168.100.222:554/Streaming/Channels/1?transportmode=unicast",320,240,NULL);
//	return RTSP_StartStream("rtsp://192.168.1.212:8556/PSIA/Streaming/channels/2?videoCodecType=H.264",NULL);
//}

int _tmain(int argc, _TCHAR* argv[])
{

	//RTSP_StartStream(url, AVDataCallBack, (void *)this);
	//CreateThread(0,0,OpenStream,NULL,0,NULL);
	//rtsp://192.168.0.222:554/Streaming/Channels/1?transportmode=unicast
	//int re1 =RTSP_StartStream("rtsp://admin:12345@192.168.0.222:554/h264/ch1/main/av_stream",NULL);
	//if(re1 >= 0){
	//	RTSP_SetCallBack(AVDataCallBackFun1,re1);
	//}


	//int num = 0;
	//while(1){
	//	Sleep(100);	
	//	if(num == 50){
	//		RTSP_SetCallBack(NULL,re1);
	//		RTSP_StopStream(re1);
	//		//RTSP_CloseAllStream();
	//		break;
	//	}
	//	num++;
	//};
	//num = 0;

	//re1 =RTSP_StartStream("rtsp://admin:12345@192.168.0.222:554/h264/ch1/main/av_stream",NULL);
	//if(re1 >= 0){
	//	RTSP_SetCallBack(AVDataCallBackFun1,re1);
	//}

	//while(1){
	//	Sleep(100);	
	//	if(num == 50){
	//		RTSP_SetCallBack(NULL,re1);
	//		RTSP_StopStream(re1);
	//		//RTSP_CloseAllStream();
	//		break;
	//	}
	//	num++;
	//};
	//num = 0;
	//
	//re1 =RTSP_StartStream("rtsp://admin:12345@192.168.0.222:554/h264/ch1/main/av_stream",NULL);
	//if(re1 >= 0){
	//	RTSP_SetCallBack(AVDataCallBackFun1,re1);
	//}

	//while(1){
	//	Sleep(100);	
	//	if(num == 50){
	//		RTSP_SetCallBack(NULL,re1);
	//		RTSP_StopStream(re1);
	//		//RTSP_CloseAllStream();
	//		break;
	//	}
	//	num++;
	//};
	//num = 0;

	int re1 =RTSP_StartStream("rtsp://admin:12345@192.168.0.222:554/h264/ch1/main/av_stream",NULL);
	if(re1 >= 0){
		RTSP_SetCallBack(AVDataCallBackFun1,re1);
	}

	while(1){
		Sleep(100);	
		
	};
	return 0;
}

