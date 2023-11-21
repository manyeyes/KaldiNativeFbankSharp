# KaldiNativeFbankSharp
## 介绍
[kaldi-native-fbank](https://github.com/csukuangfj/kaldi-native-fbank "kaldi-native-fbank")是一个c++项目，可以帮助你获得语音信号的fbank特征。

KNFWrapper是对kaldi-native-fbank的c-api接口的封装，通过dllexport方法导出共享库（在windows平台默认导出dll），支持x86,x64。

KaldiNativeFbankSharp是对kaldi-native-fbank-dll的封装。

## 用途
在c#的项目中引用KaldiNativeFbankSharp，可在包括但不限于ASR的项目中快速计算音频fbank特征。

## 调用方法
参数参考——KaldiNativeFbankSharp.OnlineFbank类的构造函数：
```csharp
public OnlineFbank(float dither, bool snip_edges, float sample_rate, int num_bins, float frame_shift = 10.0f, float frame_length = 25.0f, float energy_floor = 0.0f, bool debug_mel = false, string window_type = "hamming")
// window_type (string): Type of window ('hamming'|'hanning'|'povey'|'rectangular'|'blackman')
```
以下为示例代码，请根据项目需要配置参数：dither，snip_edges，sample_rate，num_bins,window_type ……
```csharp
//添加项目引用
using KaldiNativeFbankSharp;
//初始化OnlineFbank
OnlineFbank _onlineFbank = new OnlineFbank(
                dither: 0,
                snip_edges: false,
                sample_rate: 16000,
                num_bins: 80
                );
//传入音频samples,获取特征
public float[] GetFbank(float[] samples)
{
     float[] fbanks = _onlineFbank.GetFbank(samples);
     return fbanks;
}
```

