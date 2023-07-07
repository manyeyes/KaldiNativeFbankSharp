# KaldiNativeFbankSharp
## 介绍
[kaldi-native-fbank](https://github.com/csukuangfj/kaldi-native-fbank "kaldi-native-fbank")是一个c++项目，可以帮助你获得语音信号的fbank特征。

KNFWrapper是对kaldi-native-fbank的c-api接口的封装，通过dllexport方法导出共享库（在windows平台默认导出dll），支持x86,x64。

KaldiNativeFbankSharp是对kaldi-native-fbank-dll的封装。

## 用途
在c#的项目中引用KaldiNativeFbankSharp，可在包括但不限于ASR的项目中快速计算音频fbank特征。

## 调用方法
以下为示例代码，请根据项目需要配置参数：dither，snip_edges，sample_rate，num_bins ……
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

