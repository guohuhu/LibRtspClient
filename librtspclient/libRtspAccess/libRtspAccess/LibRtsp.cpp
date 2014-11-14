#include "StdAfx.h"
#include "LibRtsp.h"

#include <stdlib.h>
#include <time.h>
#include <vector>
#include "libRtspAccess.h"


static bool bPlaying = false;
static bool bStop = true;
static bool bRestart = false;
static time_t gLastTime = 0;

static int g_last_w = 0;
static int g_last_h = 0;

/*static int g_stream_num = 0;*/

HANDLE gPlay_tid = NULL;
HANDLE gLostDetect_tid = NULL;
//unsigned char * CLibRtsp::imageBufPtr = NULL;

extern RTSPClient* gRtspClient;
extern TaskScheduler* gScheduler;
extern UsageEnvironment* gEnv;
//extern std::vector<gParam> gParamVector;

extern AVDataCallBackFuction gAVDataFun;
extern void* gUserData;

extern char eventLoopWatchVariable;
extern char chRtspUrl[256];

extern STREAM_OVER_PROTOCOL gStream_Over_Protocol;

AVDataCallBackFuction gAVDataCallbackFun;

CLibRtsp* gThis = NULL;
std::vector<gParam> CLibRtsp::gParamVector;
static int fileNameNum = 0;

DWORD WINAPI Rtsp_Play_Thread(LPVOID pParam)
{
	while(true)
	{
		Sleep(500);
		if (eventLoopWatchVariable == 0 && gEnv != NULL)
		{
			// All subsequent activity takes place within the event loop:
			//OutputDebugStringA("Start Loop\n");
			gEnv->taskScheduler().doEventLoop(&eventLoopWatchVariable);
			//OutputDebugStringA("Stop Loop\n");
			// This function call does not return, unless, at some point in time, "eventLoopWatchVariable" gets set to something non-zero.

		}
		
	}

	
	return 0;
}



DWORD WINAPI Lost_Detect_Thread(LPVOID pParam)
{
	CLibRtsp* pThis = (CLibRtsp*)pParam;
	while(true)
	{
		Sleep(500);
		if (gLastTime == 0)
		{
			continue;
		}

		if (bStop == true)
		{
			continue;
		}

		if (bRestart)
		{
			static time_t RestartTime = 0;
			if (RestartTime == 0 || time(NULL) - RestartTime > 10)
			{
				RestartTime = time(NULL);
				gLastTime = time(NULL);
				bRestart = false;

				OutputDebugStringA("分辨率已经修改!\n");
				pThis->RestartPlay();
			}
		}

		else if (time(NULL) - gLastTime > 10 || time(NULL) < gLastTime )
		{
			gLastTime = time(NULL);
			OutputDebugStringA("已经10秒钟未收到RTP流!\n");
			pThis->RestartPlay();
		}

	}
	
	return 0;
}



void CALLBACK LostDetectCallBack(time_t LastTime)
{
// 	if (bPauseCallback == false)
// 	{
		gLastTime = LastTime;
/*	}*/
	
	
	
	// 	char abc[64] = {0};
	// 	sprintf(abc, "%ld\n", LastTime);
	// 	OutputDebugStringA(abc);
}




CLibRtsp::CLibRtsp(void)
{
	
	//imageBufPtr = (unsigned char *)malloc(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (m_pCodecCtx->width * m_pCodecCtx->height * 3));
	//memset(imageBufPtr,0,sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (m_pCodecCtx->width * m_pCodecCtx->height * 3));
	gImagePtr = NULL;
	gGetImageFlag = false;
}

CLibRtsp::~CLibRtsp(void)
{
	int i;
	eventLoopWatchVariable = 1;
	Sleep(100);
	for(i = 0; i < gParamVector.size();i++){
		avcodec_close(gParamVector.at(i).m_pCodecCtx);
		av_free(gParamVector.at(i).m_pCodecCtx);
		sws_freeContext(gParamVector.at(i).m_pCtx);
		sws_freeContext(gParamVector.at(i).m_pCtx1);
		//gParamVector.at(i).mrtspcli->~RTSPClient();
	}
}

int CLibRtsp::GetBmpImage(unsigned char *ptr)
{
	//if(ptr != NULL){
		//EnterCriticalSection(&g_img_section);
		int imageLen = 0;
		ptr = (unsigned char *)malloc(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (m_pCodecCtx->width * m_pCodecCtx->height * 3));
		gImagePtr = ptr;
		gGetImageFlag = true;
		imageLen = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (m_pCodecCtx->width * m_pCodecCtx->height * 3);
			//memcpy(ptr,imageBufPtr,sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (m_pCodecCtx->width * m_pCodecCtx->height * 3));
		return imageLen;
		//LeaveCriticalSection(&g_img_section);
	//}
}

int CLibRtsp::FreeImageBuffer()
{
	if(gImagePtr != NULL){
		free(gImagePtr);
		gImagePtr = NULL;
		return 0;
	}
	return 1;
}

bool saveAsBitmapFile(AVFrame *pFrameRGB, int width, int height)
{
	FILE *pFile = NULL;
	BITMAPFILEHEADER bmpheader; 
	BITMAPINFO bmpinfo; 

	uint8_t *buffer = pFrameRGB->data[0]; // got the raw RGB24 data

	char filePath[512] = {0};
	int bpp = 24;

	// open file
	sprintf(filePath, "%d.bmp",fileNameNum);
	fileNameNum++;
	pFile = fopen(filePath, "wb");
	if (!pFile)
		return false;

	bmpheader.bfType = ('M' <<8)|'B'; 
	bmpheader.bfReserved1 = 0; 
	bmpheader.bfReserved2 = 0; 
	bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER); 
	bmpheader.bfSize = bmpheader.bfOffBits + width*height*bpp/8; 

	bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
	bmpinfo.bmiHeader.biWidth = width; 
	bmpinfo.bmiHeader.biHeight = -height; //reverse the image
	bmpinfo.bmiHeader.biPlanes = 1; 
	bmpinfo.bmiHeader.biBitCount = bpp;
	bmpinfo.bmiHeader.biCompression = BI_RGB; 
	bmpinfo.bmiHeader.biSizeImage = 0; 
	bmpinfo.bmiHeader.biXPelsPerMeter = 100; 
	bmpinfo.bmiHeader.biYPelsPerMeter = 100; 
	bmpinfo.bmiHeader.biClrUsed = 0; 
	bmpinfo.bmiHeader.biClrImportant = 0; 

	fwrite(&bmpheader, sizeof(BITMAPFILEHEADER), 1, pFile);
	fwrite(&bmpinfo.bmiHeader, sizeof(BITMAPINFOHEADER), 1, pFile);

	for (int h=0; h<height; h++)
	{
		for (int w=0; w<width; w++)
		{
			fwrite(buffer+2, 1, 1, pFile);	// B
			fwrite(buffer+1, 1, 1, pFile);	// G
			fwrite(buffer, 1, 1, pFile);	// R

			buffer += 3;
		}
	}
	fclose(pFile); 

	return true;
}

int CLibRtsp::DecodeFrameDataToRGB(unsigned __int8* data,int iWidth,int iHeight,int iDataLen,char *pRtspUrl)
{
	int frameFinished;
	
	if(pRtspUrl != NULL && data != NULL){
		//CLibRtsp *pThis = (CLibRtsp *)gUserData;
		int i;
		std::string RtspUrlStr(pRtspUrl);
		for(i = 0; i < gParamVector.size();i++){
			if(RtspUrlStr.find(gParamVector.at(i).RtspUrl) == 0 && gParamVector.at(i).dataCallBack != NULL){
				AVFrame * m_pFrame = avcodec_alloc_frame();
				memset(m_pFrame, 0, sizeof(AVFrame));

				gParamVector.at(i).m_pCodecCtx->width = iWidth;
				gParamVector.at(i).m_pCodecCtx->height = iHeight;
				gParamVector.at(i).m_pCodecCtx->coded_width = iWidth;
				gParamVector.at(i).m_pCodecCtx->coded_height = iHeight;
				gParamVector.at(i).m_pCodecCtx->pix_fmt = PIX_FMT_YUV420P;
				int ret = -1;
				if(gParamVector.at(i).dataCallBack != NULL){
					ret = avcodec_decode_video(gParamVector.at(i).m_pCodecCtx, m_pFrame, &frameFinished, data, iDataLen);
					gParamVector.at(i).m_pCtx = sws_getContext(gParamVector.at(i).m_pCodecCtx->coded_width,gParamVector.at(i).m_pCodecCtx->coded_height,gParamVector.at(i).m_pCodecCtx->pix_fmt,gParamVector.at(i).m_pCodecCtx->coded_width, gParamVector.at(i).m_pCodecCtx->coded_height,PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
				}
				if(ret >= 0 && frameFinished > 0 && gParamVector.at(i).dataCallBack != NULL)
				{
					AVFrame *pFrameRGB;
					pFrameRGB = avcodec_alloc_frame();
					uint8_t *out_buffer;
					out_buffer=new uint8_t[avpicture_get_size(PIX_FMT_RGB24, gParamVector.at(i).m_pCodecCtx->coded_width, gParamVector.at(i).m_pCodecCtx->coded_height)];
					avpicture_fill((AVPicture *)pFrameRGB, out_buffer, PIX_FMT_RGB24, gParamVector.at(i).m_pCodecCtx->coded_width, gParamVector.at(i).m_pCodecCtx->coded_height);
					//avpicture_fill(&pFrameBGR, m_pFrame->data, PIX_FMT_RGB24, m_pCodecCtx->coded_width, m_pCodecCtx->coded_height);
					gParamVector.at(i).m_pCtx1 = sws_getContext(gParamVector.at(i).m_pCodecCtx->coded_width, gParamVector.at(i).m_pCodecCtx->coded_height,gParamVector.at(i).m_pCodecCtx->pix_fmt,gParamVector.at(i).m_pCodecCtx->coded_width, gParamVector.at(i).m_pCodecCtx->coded_height,PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
					sws_scale(gParamVector.at(i).m_pCtx1,m_pFrame->data, m_pFrame->linesize,0, gParamVector.at(i).m_pCodecCtx->coded_height,pFrameRGB->data, pFrameRGB->linesize);
					//saveAsBitmapFile(pFrameRGB,iWidth,iHeight);
					if(gParamVector.at(i).dataCallBack != NULL)
						gParamVector.at(i).dataCallBack((char *)pFrameRGB->data[0], iWidth*iHeight*3, iWidth, iHeight);
					av_free(pFrameRGB);
					pFrameRGB = NULL;
					delete out_buffer;
					out_buffer = NULL;
					sws_freeContext(gParamVector.at(i).m_pCtx1);
					gParamVector.at(i).m_pCtx1 = NULL;

				}
				sws_freeContext(gParamVector.at(i).m_pCtx);
				gParamVector.at(i).m_pCtx = NULL;
				av_free(m_pFrame);
				m_pFrame = NULL;
			}
		}
	}

	//delete [] dataPtr;
	return 0;
}

int CLibRtsp::DecodeFrameData(unsigned __int8* data,int iWidth,int iHeight,int iDataLen,char *pRtspUrl)
{
	int frameFinished;
	//unsigned char *dataPtr = new unsigned char[iDataLen];
	//memcpy(dataPtr,data,iDataLen);
	if(gUserData != NULL && data != NULL){
		CLibRtsp *pThis = (CLibRtsp *)gUserData;
		int i;
		for(i = 0; i < gParamVector.size();i++){
			if(gParamVector.at(i).RtspUrl.compare(pRtspUrl) == 0 && gParamVector.at(i).dataCallBack != NULL){
				AVFrame * m_pFrame = avcodec_alloc_frame();
				memset(m_pFrame, 0, sizeof(AVFrame));

				pThis->m_pCodecCtx->width = iWidth;
				pThis->m_pCodecCtx->height = iHeight;
				pThis->m_pCodecCtx->coded_width = iWidth;
				pThis->m_pCodecCtx->coded_height = iHeight;
				pThis->m_pCodecCtx->pix_fmt = PIX_FMT_YUV420P;
				
				int ret = avcodec_decode_video(pThis->m_pCodecCtx, m_pFrame, &frameFinished, data, iDataLen);
				pThis->m_pCtx = sws_getContext(pThis->m_pCodecCtx->coded_width,pThis->m_pCodecCtx->coded_height,pThis->m_pCodecCtx->pix_fmt,pThis->m_pCodecCtx->coded_width, pThis->m_pCodecCtx->coded_height,PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
				if(ret >= 0 && frameFinished > 0)
				{
					AVFrame *pFrameYUV;
					//AVPicture pFrameBGR;
					pFrameYUV = avcodec_alloc_frame();
					uint8_t *out_buffer;
					out_buffer=new uint8_t[avpicture_get_size(PIX_FMT_YUV420P, pThis->m_pCodecCtx->coded_width, pThis->m_pCodecCtx->coded_height)];
					avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pThis->m_pCodecCtx->coded_width, pThis->m_pCodecCtx->coded_height);

					sws_scale(pThis->m_pCtx,m_pFrame->data, m_pFrame->linesize,0, pThis->m_pCodecCtx->coded_height,pFrameYUV->data, pFrameYUV->linesize);
					int picSize = pThis->m_pCodecCtx->height * pThis->m_pCodecCtx->width;
					int newSize = picSize * 1.5;
					unsigned char *buf = new unsigned char[newSize];
					int height = pThis->m_pCodecCtx->coded_height;
					int width = pThis->m_pCodecCtx->coded_width;
					int a=0,j;

					for (j=0; j<height; j++)   
					{   
						memcpy(buf+a,pFrameYUV->data[0] + j * pFrameYUV->linesize[0], width);   
						a+=width;
					}   
					for (j=0; j<height/2; j++)   
					{   
						memcpy(buf+a,pFrameYUV->data[1] + j * pFrameYUV->linesize[1], width/2);   
						a+=width/2;   
					}   
					for (j=0; j<height/2; j++)   
					{   
						memcpy(buf+a,pFrameYUV->data[2] + j * pFrameYUV->linesize[2], width/2);   
						a+=width/2;   
					}  

					gParamVector.at(i).dataCallBack((char *)buf, newSize, iWidth, iHeight);
					delete [] buf;
					av_free(pFrameYUV);
					delete out_buffer;
				}
				
				av_free(m_pFrame);
			}
		}
	}

	//delete [] dataPtr;
	return 0;
	
}

int CALLBACK CLibRtsp::AVDataCallBackFun(int iAVSelect, char* pAVData, int iDataLen, int iWidth, int iHeight, int SessionID, void* pUserData)
{
	printf("avdata callback\n");
	static bool bCallback = false;

	//CLibRtsp* pThis = (CLibRtsp*)pUserData;
	//RTSPClient *pRtspClient = (RTSPClient *)pUserData;
	char *mRtspUrl = (char *)pUserData;
#if 0
	if (bCallback == false && iAVSelect == 0)
	{
		if (g_last_w == 0 || g_last_h == 0)
		{
			g_last_w = iWidth;
			g_last_h = iHeight;

			bCallback = true;
		}
	}

	if (iAVSelect == 0)
	{
	
		if ((g_last_w != iWidth || g_last_h != iHeight) && (g_last_h != 0 && g_last_w != 0))
		{
			bCallback = false;
 			
			if (iWidth != 0 && iHeight != 0)
			{
				bRestart = true;
			}
			
// 			Sleep(100);
// 			bPauseCallback = false;

			OUTPUT_DEBUG_STRING("last %d %d now %d %d\n", g_last_w, g_last_h, iWidth, iHeight);

		}
		else if (iWidth == 0 || iHeight == 0)
		{
			bCallback = false;
		}
		else
		{
			
			bCallback = true;
		}
				
	}
	else if (iAVSelect == 1)
	{
		bCallback = true;
	}
#endif

	if (iAVSelect == 0 )
	{	
		//unsigned __int8* data = (unsigned __int8*)av_malloc( iDataLen + GBUFFER_PADDDING_SIZE);
		//memset(data,0,iDataLen + GBUFFER_PADDDING_SIZE);
		//memcpy(data,pAVData,iDataLen);
		//if( iDataLen > 26)
		//DecodeFrameData((unsigned __int8*)pAVData,iWidth,iHeight,iDataLen,pRtspClient);
		DecodeFrameDataToRGB((unsigned __int8*)pAVData,iWidth,iHeight,iDataLen,mRtspUrl);
		//av_free(data);
	}
	
//	if(pThis->gGetImageFlag){
//		unsigned __int8* data = (unsigned __int8*)av_malloc( iDataLen + GBUFFER_PADDDING_SIZE);
//		memset(data,0,iDataLen + GBUFFER_PADDDING_SIZE);
//		memcpy(data,pAVData,iDataLen);
//		pThis->DecodeFrameData(data,iWidth,iHeight,iDataLen,pUserData);
//		av_free(data);
//		pThis->gGetImageFlag = false;
//	}
	
	return 0;
}


void CLibRtsp::RestartPlay()
{
	char RtspUrl[256] = {0};
	AVDataCallBackFuction AVDataFun = gAVDataCallbackFun;
	void* UserData = gUserData;
	STREAM_OVER_PROTOCOL Stream_Over_Protocol = gStream_Over_Protocol;
	strcpy(RtspUrl, chRtspUrl);
	OutputDebugStringA("RestartPlay\n");
	StopStream(false);
	Sleep(500);
	StartStream(RtspUrl,UserData, Stream_Over_Protocol);
}

void CLibRtsp::SetAvDataCallBack(YUVDataCallBackFuction AVDataFun,int id)
{
	//gAVDataCallbackFun = AVDataFun;
	if(id != -1){
		//if(gParamVector.at(id).dataCallBack == NULL){
		gParamVector.at(id).dataCallBack = AVDataFun;
		//}
	}
}

int CLibRtsp::StartStream(const char* pRtspUrl,void* pUserData, STREAM_OVER_PROTOCOL Stream_Over_Protocol)
{

//	if (bPlaying)
//	{
//		return -1;
//	}


//	if (pRtspUrl == NULL || 
//		AVDataFun == NULL || 
//		pUserData == NULL || 
//		Stream_Over_Protocol > STREAMING_OVER_TCP || 
//		Stream_Over_Protocol < STREAMING_OVER_UDP)
//	{
//		return -1;
//	}

	gThis = this;
	bPlaying = true;
	bStop = false;
	eventLoopWatchVariable = 0;

	

	gStream_Over_Protocol = Stream_Over_Protocol;
	gAVDataFun = AVDataCallBackFun;
	//gAVDataFun = AVDataFun;
	//gAVDataCallbackFun = AVDataFun;
	//gUserData = pUserData;
	gUserData = this;

	strcpy(chRtspUrl, pRtspUrl);

	if (gScheduler == NULL)
	{
		gScheduler = BasicTaskScheduler::createNew();
	}
	
	if (gEnv == NULL)
	{
	gEnv = BasicUsageEnvironment::createNew(*gScheduler);
	}
	
	//openURL(*env, NULL, "rtsp://192.168.0.214:8557/PSIA/Streaming/channels/2?videoCodecType=H.264");

	RTSPClient* mRtspClient = (RTSPClient*)openURL(*gEnv, NULL, chRtspUrl, LostDetectCallBack);
	if(mRtspClient != NULL){
		gParam tmpGParam;
		tmpGParam.mrtspcli = mRtspClient;
		tmpGParam.dataCallBack = NULL;
		tmpGParam.RtspUrl.assign(pRtspUrl);
		tmpGParam.m_pCodec = NULL;
		tmpGParam.m_pCodecCtx = NULL;
		tmpGParam.m_pCtx = NULL;
		tmpGParam.m_pCtx1 = NULL;
		avcodec_register_all();
		av_register_all();
		tmpGParam.m_pCodec = avcodec_find_decoder(CODEC_ID_H264);
		tmpGParam.m_pCodecCtx = avcodec_alloc_context();
		avcodec_open(tmpGParam.m_pCodecCtx,tmpGParam.m_pCodec);
		gParamVector.push_back(tmpGParam);
	}
	
//	gEnv->taskScheduler().doEventLoop(&eventLoopWatchVariable);
	if (gPlay_tid == NULL)
	{
		gPlay_tid = CreateThread(0, 0, Rtsp_Play_Thread, (void*)this, 0, NULL);
	}
//	if (gLostDetect_tid == NULL)
//	{
//		gLostDetect_tid = CreateThread(0, 0, Lost_Detect_Thread, (void*)this, 0, NULL);
//	}
	
	
	//gEnv->taskScheduler().doEventLoop(&eventLoopWatchVariable);
	
	return gParamVector.size() - 1;
}

int CLibRtsp::CloseAllStream()
{
	int i;
	for(i = 0;i < gParamVector.size();i++){
		gParamVector.at(i).dataCallBack = NULL;
	}
	eventLoopWatchVariable = 1;
	for(i = 0;i < gParamVector.size();i++){
		if (gParamVector.at(i).mrtspcli != NULL){
			shutdownStream(gParamVector.at(i).mrtspcli);
			gParamVector.at(i).mrtspcli = NULL;
			avcodec_close(gParamVector.at(i).m_pCodecCtx);
			av_free(gParamVector.at(i).m_pCodecCtx);
			gParamVector.at(i).m_pCodecCtx = NULL;
			sws_freeContext(gParamVector.at(i).m_pCtx);
			gParamVector.at(i).m_pCtx = NULL;
			sws_freeContext(gParamVector.at(i).m_pCtx1);
			gParamVector.at(i).m_pCtx1 = NULL;
		}
	}
	if (gAVDataFun != NULL)
	{
		gAVDataFun = NULL;
	}

	if (gUserData != NULL)
	{
		gUserData = NULL;
	}

	memset(chRtspUrl, 0, sizeof(chRtspUrl));

	bPlaying = false;
	return 0;
}

int CLibRtsp::StopStream(int id)
{
// 	g_stream_num --;
// 
// 	if (g_stream_num < 0)
// 	{
// 		OutputDebugStringA("g_stream_num < 0\n");
// 		return -1;
// 	}
	

//	if (bUninit)
//	{
//		g_last_w = g_last_h = 0;
//
//		gLastTime = 0;

//		bStop = true;
//	}
	
	
	int i;
	//int gParamVectorLen = gParamVector.size();
	if (gParamVector.at(id).mrtspcli != NULL)
	{
				//g_last_w = g_last_h = 0;
		
				//gLastTime = 0;
				//bStop = true;
//		gParamVectorLen--;
//		if(gParamVectorLen == 0){
//			eventLoopWatchVariable = 1;
//		}
		shutdownStream(gParamVector.at(id).mrtspcli);
		//gParamVector.at(id).mrtspcli = NULL;
	}

//	if(gParamVectorLen == 0){
//		gPlay_tid = NULL;
		//delete gScheduler;
//		gScheduler = NULL;
		//delete gEnv;
//		gEnv = NULL;
//	}

	if (gAVDataFun != NULL)
	{
		gAVDataFun = NULL;
	}

	if (gUserData != NULL)
	{
		gUserData = NULL;
	}

	//memset(chRtspUrl, 0, sizeof(chRtspUrl));

	//bPlaying = false;
	
	//avcodec_close(gParamVector.at(id).m_pCodecCtx);
	//av_free(gParamVector.at(id).m_pCodecCtx);
	//gParamVector.at(id).m_pCodecCtx = NULL;
	//sws_freeContext(gParamVector.at(id).m_pCtx);
	//gParamVector.at(id).m_pCtx = NULL;
	//sws_freeContext(gParamVector.at(id).m_pCtx1);
	//gParamVector.at(id).m_pCtx1 = NULL;
	//gParamVector.erase(gParamVector.begin()+id,gParamVector.begin()+id+1);
		//gParamVector.at(i).mrtspcli->~RTSPClient();
	
	return 0;

}