// See https://github.com/manyeyes for more information
// Copyright (c)  2023 by manyeyes
using KaldiNativeFbankSharp.Struct;
using System.Runtime.InteropServices;

namespace KaldiNativeFbankSharp.DLL
{
    internal static partial class KaldiNativeFbank
    {
        private const string dllName = @"kaldi-native-fbank";

        [DllImport(dllName, EntryPoint = "GetFbankOptions", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr GetFbankOptions(float dither, bool snip_edges, float sample_rate, int num_bins, int num_ceps = 40, float frame_shift = 10.0f, float frame_length = 25.0f, float energy_floor = 0.0f, bool debug_mel = false, string window_type = "hamming", string feature_type = "fbank");

        [DllImport(dllName, EntryPoint = "GetOnlineFbank", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        internal static extern KnfOnlineFeature GetOnlineFbank(IntPtr opts);

        [DllImport(dllName, EntryPoint = "AcceptWaveform", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void AcceptWaveform(KnfOnlineFeature knfOnlineFeature, float sample_rate, float[] samples, int samples_size);

        [DllImport(dllName, EntryPoint = "InputFinished", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void InputFinished(KnfOnlineFeature knfOnlineFeature);

        [DllImport(dllName, EntryPoint = "GetNumFramesReady", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetNumFramesReady(KnfOnlineFeature knfOnlineFeature);

        [DllImport(dllName, EntryPoint = "GetFbank", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void GetFbank(KnfOnlineFeature knfOnlineFeature, int currFrameIndex, ref FbankData pData);

        [DllImport(dllName, EntryPoint = "GetFbanks", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void GetFbanks(KnfOnlineFeature knfOnlineFeature, int lastFrameIndex, ref FbankDatas fbankDatas);

    }
}
