#include "StdAfx.h"
#include "VideoDecode.h"

VideoDecode::VideoDecode(void)
{
	m_pCtx = NULL;
	memset(&m_Picture, 0 , sizeof(AVPicture));
	imageNum = 0;
}

VideoDecode::~VideoDecode(void)
{
}

VideoDecode::VideoInit(int image_w, int image_h)
{
	avcodec_register_all();
	av_register_all();

	m_pCodec = avcodec_find_decoder(CODEC_ID_H264);
	if (!m_pCodec)
		return -1;
	m_pCodecCtx = avcodec_alloc_context();

	avcodec_open(m_pCodecCtx, m_pCodec);
	m_pCodecCtx->height = image_h;
	m_pCodecCtx->width = image_w;
	m_pCodecCtx->pix_fmt = PIX_FMT_YUV420P;
	// Allocate video frame
	m_pCodecCtx->coded_height = m_ImageHeight;
	m_pCodecCtx->coded_width = m_ImageWidth;

	m_pCtx = sws_getContext(m_pCodecCtx->width, m_pCodecCtx->height,m_pCodecCtx->pix_fmt,
		m_pCodecCtx->width, m_pCodecCtx->height,
		PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

}

bool VideoDecode::saveAsBitmapFile(AVFrame *pFrameRGB, int width, int height)
{
	FILE *pFile = NULL;
	BITMAPFILEHEADER bmpheader; 
	BITMAPINFO bmpinfo; 

	uint8_t *buffer = pFrameRGB->data[0]; // got the raw RGB24 data

	char filePath[512] = {0};
	int bpp = 24;

	// open file
	sprintf(filePath, "rtspImage\\%d.bmp",imageNum);
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
	imageNum++;

	char fileName[512] = {0};
	sprintf(fileName,"%s.bmp",this->faceName.c_str());
	list<string>::iterator it;

	for(it = FileNameList.begin();it != FileNameList.end();it++){
		if(it->compare(fileName) == 0){
			break;
		}
	}
	if(it == FileNameList.end()){
		FileNameList.push_back(fileName);
		addXmlElement(string(fileName),string(fileName));
	}

	return true;
}

int CALLBACK VideoDecode::AVDataCallBack(int iAVSelect, char* pAVData, int iDataLen, int iWidth, int iHeight, int SessionID, void* pUserData)
{
	int frameFinished;

	m_pFrame = avcodec_alloc_frame();
	int ret = avcodec_decode_video(m_pCodecCtx, m_pFrame, &frameFinished, pAVData, iDataLen);
	if(ret >= 0 && frameFinished > 0)
	{
		AVFrame *pFrameRGB;
		//AVPicture pFrameBGR;
		pFrameRGB = avcodec_alloc_frame();
		uint8_t *out_buffer;
		out_buffer=new uint8_t[avpicture_get_size(PIX_FMT_RGB24, m_pCodecCtx->coded_width, m_pCodecCtx->coded_height)];
		avpicture_fill((AVPicture *)pFrameRGB, out_buffer, PIX_FMT_RGB24, m_pCodecCtx->coded_width, m_pCodecCtx->coded_height);
		//avpicture_fill(&pFrameBGR, m_pFrame->data, PIX_FMT_RGB24, m_pCodecCtx->coded_width, m_pCodecCtx->coded_height);
		m_pCtx1 = sws_getContext(m_pCodecCtx->coded_width, m_pCodecCtx->coded_height,m_pCodecCtx->pix_fmt,m_pCodecCtx->coded_width, m_pCodecCtx->coded_height,PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
		sws_scale(m_pCtx1,pAVData, m_pFrame->linesize,0, m_pCodecCtx->coded_height,pFrameRGB->data, pFrameRGB->linesize);
		//sws_scale(m_pCtx1,m_pFrame->data, m_pFrame->linesize,0, m_pCodecCtx->coded_height,pFrameBGR.data, pFrameBGR.linesize);
		saveAsBitmap(pFrameRGB,m_pCodecCtx->coded_width,m_pCodecCtx->coded_height,0);
		av_free(pFrameRGB);
		free(out_buffer);
	}

}	