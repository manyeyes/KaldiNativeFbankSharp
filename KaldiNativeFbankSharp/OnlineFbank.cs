// See https://github.com/manyeyes for more information
// Copyright (c)  2023 by manyeyes
using KaldiNativeFbankSharp.DLL;
using KaldiNativeFbankSharp.Struct;
using System.Runtime.InteropServices;

namespace KaldiNativeFbankSharp
{
    public class OnlineFbank : OnlineBase
    {
        private float _sample_rate = 16000.0F;
        private int _num_bins = 80;
        private int _last_frame_index = 0;

        public OnlineFbank(float dither, bool snip_edges, float sample_rate, int num_bins, int num_ceps = 40, float frame_shift = 10.0f, float frame_length = 25.0f, float energy_floor = 0.0f, bool debug_mel = false, string window_type = "hamming", string feature_type = "fbank")
        {
            _sample_rate = sample_rate;
            _num_bins = num_bins;
            this._opts = KaldiNativeFbank.GetFbankOptions(
                 dither: dither,
                 snip_edges: snip_edges,
                 sample_rate: sample_rate,
                 num_bins: num_bins,
                 num_ceps: num_ceps,
                 frame_shift: frame_shift,
                 frame_length: frame_length,
                 energy_floor: energy_floor,
                 debug_mel: debug_mel,
                 window_type: window_type,
                 feature_type: feature_type
                 );
            this._knfOnlineFeature = KaldiNativeFbank.GetOnlineFbank(this._opts);
        }

        /// <summary>
        /// Get one frame at a time
        /// </summary>
        /// <param name="samples"></param>
        /// <returns></returns>
        public float[] GetFbank(float[] samples)
        {
            KaldiNativeFbank.AcceptWaveform(_knfOnlineFeature, _sample_rate, samples, samples.Length);
            int framesNum = KaldiNativeFbank.GetNumFramesReady(_knfOnlineFeature);
            int n = framesNum - _last_frame_index;
            float[] fbanks = new float[n * _num_bins];
            for (int i = _last_frame_index; i < framesNum; i++)
            {
                FbankData fbankData = new FbankData();
                KaldiNativeFbank.GetFbank(_knfOnlineFeature, i, ref fbankData);
                float[] _fbankData = new float[fbankData.data_length];
                Marshal.Copy(fbankData.data, _fbankData, 0, fbankData.data_length);
                Array.Copy(_fbankData, 0, fbanks, (i - _last_frame_index) * _num_bins, _fbankData.Length);
                fbankData.data = IntPtr.Zero;
                _fbankData = null;
            }
            _last_frame_index += n;
            samples = null;
            return fbanks;
        }      

        /// <summary>
        /// Get all current frames each time
        /// Faster than GetFbank
        /// </summary>
        /// <param name="samples"></param>
        /// <returns></returns>
        public float[] GetFbankIndoor(float[] samples)
        {
            KaldiNativeFbank.AcceptWaveform(_knfOnlineFeature, _sample_rate, samples, samples.Length);
            int framesNum = KaldiNativeFbank.GetNumFramesReady(_knfOnlineFeature);
            FbankDatas fbankDatas = new FbankDatas();
            KaldiNativeFbank.GetFbanks(_knfOnlineFeature, _last_frame_index, ref fbankDatas);
            float[] _fbankDatas = new float[fbankDatas.data_length];
            Marshal.Copy(fbankDatas.data, _fbankDatas, 0, fbankDatas.data_length);
            _last_frame_index = framesNum - 1;
            samples = null;
            return _fbankDatas;
        }

        public void InputFinished()
        {
            KaldiNativeFbank.InputFinished(_knfOnlineFeature);
        }

        protected override void Dispose(bool disposing)
        {
            if (!disposing)
            {
                this._opts = IntPtr.Zero;
                this._knfOnlineFeature.impl = IntPtr.Zero;
                this._disposed = true;
                base.Dispose();
            }
        }
    }
}