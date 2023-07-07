//KNFWrapper.h
#pragma once

#ifndef KALDI_NATIVE_FBANK_C_API_SAMPLECPPWRAPPER_H_
#define KALDI_NATIVE_FBANK_C_API_SAMPLECPPWRAPPER_H_

#ifdef _WIN32
#    ifdef LIBRARY_EXPORTS
#        define LIBRARY_API __declspec(dllexport)
#    else
#        define LIBRARY_API __declspec(dllimport)
#    endif
#elif
#    define LIBRARY_API
#endif


#include "feature-fbank.h"
#include "online-feature.h"
#include <list>
#include <wincrypt.h>

#ifdef __cplusplus
extern "C" {
#endif
	namespace knf
	{
		typedef struct FbankData300000
		{
			float data[300000];
			int data_length;
		}FbankData300000;

		typedef struct FbankData
		{
			const float* data;
			int data_length;
		}FbankData;

		typedef struct FbankDatas {
			float* data;
			int data_length;
		} FbankDatas;

		typedef struct KnfOnlineFbank KnfOnlineFbank;

		LIBRARY_API FbankOptions* GetFbankOptions(float dither, bool snip_edges, float sample_rate, int32_t num_bins, float frame_shift = 10.0f, float frame_length = 25.0f, float energy_floor = 0.0f, bool debug_mel = false, const char* window_type = "hamming");
		LIBRARY_API KnfOnlineFbank* GetOnlineFbank(FbankOptions* opts);
		LIBRARY_API void AcceptWaveform(KnfOnlineFbank* knfOnlineFbank, float sample_rate, float* samples, int samples_size);
		LIBRARY_API void InputFinished(KnfOnlineFbank* knfOnlineFbank);
		LIBRARY_API int32_t GetNumFramesReady(KnfOnlineFbank* knfOnlineFbank);
		LIBRARY_API void GetFbank(KnfOnlineFbank* knfOnlineFbank, int frame, FbankData* /*out*/ pData);
		LIBRARY_API void GetFbanks(KnfOnlineFbank* knfOnlineFbank, int framesNum, FbankDatas* /*out*/ pData);
		std::vector<float> GetFrames(KnfOnlineFbank* knfOnlineFbank, int framesNum);
	}
#ifdef __cplusplus
}
#endif

#endif  // KALDI_NATIVE_FBANK_C_API_KNFWRAPPER_H_