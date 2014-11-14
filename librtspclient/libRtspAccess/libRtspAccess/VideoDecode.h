#pragma once
extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

}


class VideoDecode
{
public:
	VideoDecode(void);
	~VideoDecode(void);
private:
	AVCodecContext *m_pCodecCtx;
	AVPicture m_Picture;
	AVFrame *m_pFrame;
	AVCodec *m_pCodec;
	struct SwsContext *m_pCtx;
	struct SwsContext *m_pCtx1;
	int imageNum;
	int VideoInit(int image_w, int image_h);
	bool saveAsBitmapFile(AVFrame *pFrameRGB, int width, int height);
	static int CALLBACK AVDataCallBack(int iAVSelect, char* pAVData, int iDataLen, int iWidth, int iHeight, int SessionID, void* pUserData);
};
