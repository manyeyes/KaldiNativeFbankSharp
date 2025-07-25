// See https://github.com/manyeyes for more information
// Copyright (c)  2023 by manyeyes
using System.Runtime.InteropServices;

namespace KaldiNativeFbankSharp.Struct
{
    internal struct FbankData
    {
        public IntPtr data;
        public int data_length;
    };

    internal struct FbankDatas
    {
        public IntPtr data;
        public int data_length;
    };

    internal struct KnfOnlineFeature
    {
        public IntPtr impl;
    };
}
