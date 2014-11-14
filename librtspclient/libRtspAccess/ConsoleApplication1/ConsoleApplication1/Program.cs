using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using vfw;


namespace ConsoleApplication1
{
     
    class Program
    {
        private static void AVData1(IntPtr pAVData, int iDataLen, int iWidth, int iHeight)
        {
            //StreamWriter sw = new StreamWriter(@"H:\ConsoleOutput.txt");
            //Console.SetOut(sw);
            Console.WriteLine("callback1 data len is " + iDataLen.ToString());
            //Console.WriteLine("OK!");

            //sw.Flush();
            //sw.Close();
            
            return;
        }

        private static void AVData2(IntPtr pAVData, int iDataLen, int iWidth, int iHeight)
        {
            //StreamWriter sw = new StreamWriter(@"H:\ConsoleOutput.txt");
            //Console.SetOut(sw);
            Console.WriteLine("callback2 data len is " + iDataLen.ToString());
            //Console.WriteLine("OK!");

            //sw.Flush();
            //sw.Close();

            return;
        }
       
        static void Main(string[] args)
        {
            string rtspURL1 = "rtsp://192.168.9.211/test1.264";
           RtspSDK RtspSDK = new RtspSDK();
            int iChannle1 = RtspSDK.RTSP_StartStream(rtspURL1,(IntPtr)null,STREAM_OVER_PROTOCOL.STREAMING_OVER_UDP);
            RtspSDK.RTSP_SetCallBack(new RtspSDK.AVDataCallBackFuction(AVData1),iChannle1);

            string rtspURL2 = "rtsp://admin:12345@192.168.0.222:554/h264/ch1/main/av_stream";
            int iChannle2 = RtspSDK.RTSP_StartStream(rtspURL2, (IntPtr)null, STREAM_OVER_PROTOCOL.STREAMING_OVER_UDP);
            RtspSDK.RTSP_SetCallBack(new RtspSDK.AVDataCallBackFuction(AVData2), iChannle2);

            while(true){
                System.Threading.Thread.Sleep(100);
            }
        }
    }
}
