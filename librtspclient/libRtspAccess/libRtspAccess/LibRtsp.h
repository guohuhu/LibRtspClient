#pragma once

#include "libRtspAccess.h"
#include "RtspImpletement.h"
#include <string>
#include <vector>
extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

}

#define GBUFFER_PADDDING_SIZE 32
typedef struct 
{
	AVCodec *m_pCodec;
	AVCodecContext *m_pCodecCtx;
	struct SwsContext *m_pCtx;
	struct SwsContext *m_pCtx1;
	RTSPClient* mrtspcli;
	std::string RtspUrl;
	YUVDataCallBackFuction dataCallBack;
}gParam;


class CLibRtsp
{
public:
	CLibRtsp(void);
	~CLibRtsp(void);

	void RestartPlay();
	//int StartStream(const char* pRtspUrl, AVDataCallBackFuction AVDataFun, void* pUserData, STREAM_OVER_PROTOCOL Stream_Over_Protocol);
	int StartStream(const char* pRtspUrl,void* pUserData, STREAM_OVER_PROTOCOL Stream_Over_Protocol);
	int StopStream(int id);
	int CloseAllStream();
	int GetBmpImage(unsigned char *ptr);
	int FreeImageBuffer();
	void SetAvDataCallBack(YUVDataCallBackFuction AVDataFun,int id);

	static int CALLBACK AVDataCallBackFun(int iAVSelect, char* pAVData, int iDataLen, int iWidth, int iHeight, int SessionID, void* pUserData);
	//static unsigned char *imageBufPtr;
	static int DecodeFrameData(unsigned __int8* data,int iWidth,int iHeight,int iDataLen,char *pRtspurl);
	static int DecodeFrameDataToRGB(unsigned __int8* data,int iWidth,int iHeight,int iDataLen,char *pRtspUrl);
	static std::vector<gParam> gParamVector;
private:
	AVCodec *m_pCodec;
	AVCodecContext *m_pCodecCtx;
	struct SwsContext *m_pCtx;
	struct SwsContext *m_pCtx1;
	AVPicture m_Picture;
	CRITICAL_SECTION g_img_section;
	
	
	int saveAsBitmap(AVFrame *pFrameRGB, int width, int height);
	//int DecodeFrameData(unsigned __int8* data,int width,int height,int iDataLen,,RTSPClient *pRtspClient);
	
	unsigned char *gImagePtr;
	bool gGetImageFlag;
	
};
