using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace vfw
{
    public enum STREAM_OVER_PROTOCOL
    {
        STREAMING_OVER_UDP = 0,
        STREAMING_OVER_TCP
    }

    public class RtspSDK
    {
        /// <summary>
        /// 获取rtsp 视频流 int RTSP_StartStream(const char* pRtspUrl,void* pUserData, STREAM_OVER_PROTOCOL Stream_Over_Protocol)
        /// 
        /// 如：int ret =RTSP_StartStream("rtsp://192.168.1.212:8556/PSIA/Streaming/channels/2?videoCodecType=H.264",NULL);
        /// 最后一个参数Stream_Over_Protocol默认为使用UDP，可以不填。
        /// 返回值为唯一标示一个流的客户端id。返回-1为错误。
        /// </summary>
        /// <param name="videoinfo"></param>
        /// <returns></returns>
        [DllImport("libRtspAccess.dll")]
        public static extern int RTSP_StartStream(string pRtspUrl,IntPtr pUserData, STREAM_OVER_PROTOCOL Stream_Over_Protocol);

        /// <summary>
        /// 停止rtsp视频流，断开连接。int RTSP_StopStream(int id);
        /// 传入的唯一标示一个流的客户端id。
        /// </summary>
        /// <returns></returns>
        [DllImport("libRtspAccess.dll")]
        public static extern int RTSP_StopStream(int id);

        /// <summary>
        /// 设置一个流客户端的回调函数，void RTSP_SetCallBack(AVDataCallBackFuction avDataCallBack,int id);
        /// avDataCallBack为回调函数，定义如下：
        /// typedef int (CALLBACK *AVDataCallBackFuction)(int iAVSelect, char* pAVData, int iDataLen, int iWidth, int iHeight, int SessionID, void* pUserData);
        /// id 为RTSP_StartStream的返回值。
        /// </summary>
        /// <param name="avDataCallBack"></param>
        /// <param name="id"></param>
        [DllImport("libRtspAccess.dll")]
        public static extern void RTSP_SetCallBack(AVDataCallBackFuction avDataCallBack, int id);

        /// <summary>
        /// typedef int (CALLBACK *AVDataCallBackFuction)(int iAVSelect, char* pAVData, int iDataLen, int iWidth, int iHeight, int SessionID, void* pUserData);
        /// </summary>
        /// <param name="iAVSelect"></param>
        /// <param name="pAVData"></param>
        /// <param name="iDataLen"></param>
        /// <param name="iWidth"></param>
        /// <param name="iHeight"></param>
        /// <param name="SessionID"></param>
        /// <param name="pUserData"></param>
        //public delegate void AVDataCallBackFuction(int iAVSelect, IntPtr pAVData, int iDataLen, int iWidth, int iHeight, int SessionID, IntPtr pUserData);
        public delegate void AVDataCallBackFuction(IntPtr pAVData, int iDataLen, int iWidth, int iHeight);
    }
}
