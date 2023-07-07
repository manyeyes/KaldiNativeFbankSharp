// See https://github.com/manyeyes for more information
// Copyright (c)  2023 by manyeyes
using KaldiNativeFbankSharp.DLL;
using KaldiNativeFbankSharp.Struct;
using System.Runtime.InteropServices;

namespace KaldiNativeFbankSharp
{
    public class OnlineFbank: OnlineBase
    {       
        private float _sample_rate = 16000.0F;
        private int _num_bins = 80;

        public OnlineFbank(float dither,bool snip_edges,float sample_rate,int num_bins)
        {
            _sample_rate=sample_rate;
            _num_bins=num_bins;
            this._opts = KaldiNativeFbank.GetFbankOptions(
                 dither: dither,
                 snip_edges: snip_edges,
                 sample_rate: sample_rate,
                 num_bins: num_bins
                 );
            this._knfOnlineFbank = KaldiNativeFbank.GetOnlineFbank(_opts);
        }

        public float[] GetFbank(float[] samples)
        {
            KaldiNativeFbank.AcceptWaveform(_knfOnlineFbank, _sample_rate, samples, samples.Length);
            KaldiNativeFbank.InputFinished(_knfOnlineFbank);
            int framesNum = KaldiNativeFbank.GetNumFramesReady(_knfOnlineFbank);
            float[] fbanks = new float[framesNum * _num_bins];
            for (int i = 0; i < framesNum; i++)
            {
                FbankData fbankData = new FbankData();
                KaldiNativeFbank.GetFbank(_knfOnlineFbank, i, ref fbankData);
                float[] _fbankData = new float[fbankData.data_length];
                Marshal.Copy(fbankData.data, _fbankData, 0, fbankData.data_length);
                Array.Copy(_fbankData, 0, fbanks, i * _num_bins, _fbankData.Length);
                fbankData.data = IntPtr.Zero;
                _fbankData = null;
            }
            samples = null;
            return fbanks;
        }

        public float[] GetFbankIndoor(float[] samples)
        {
            KaldiNativeFbank.AcceptWaveform(_knfOnlineFbank, _sample_rate, samples, samples.Length);
            KaldiNativeFbank.InputFinished(_knfOnlineFbank);
            int framesNum = KaldiNativeFbank.GetNumFramesReady(_knfOnlineFbank);
            FbankDatas fbankDatas = new FbankDatas();
            KaldiNativeFbank.GetFbanks(_knfOnlineFbank, framesNum, ref fbankDatas);
            float[] _fbankDatas = new float[fbankDatas.data_length];
            Marshal.Copy(fbankDatas.data, _fbankDatas, 0, fbankDatas.data_length);
            samples = null;
            return _fbankDatas;
        }

        protected override void Dispose(bool disposing)
        {
            if (!disposing)
            {
                this._opts=IntPtr.Zero;
                this._knfOnlineFbank.impl = IntPtr.Zero;
                this._disposed = true;
                base.Dispose();
            }
        }
    }
}