//KNFWrapper.h
#pragma once

#ifndef KALDI_NATIVE_FBANK_C_API_KNFWRAPPER_H_
#define KALDI_NATIVE_FBANK_C_API_KNFWRAPPER_H_

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

		struct FeatureOptions {
			float dither = 1.0f;
			bool snip_edges = true;
			float sample_rate = 16000;
			int32_t num_bins = 25;  // e.g. 25; number of triangular bins
			float frame_shift = 10.0f;   // in milliseconds.
			float frame_length = 25.0f;  // in milliseconds.
			float energy_floor = 0.0f;  // active iff use_energy==true
			bool debug_mel = false;
			int32_t num_ceps = 13;// Number of cepstra in MFCC computation (including C0)		
			std::string window_type = "povey";  // e.g. Hamming window
			std::string feature_type = "fbank";  // e.g. fbank, whisper, mfcc
			// Standby parameters
			float low_freq = 20;
			float high_freq = 0;
			float vtln_low = 100;
			float vtln_high = -500;
			bool htk_compat = false;
			bool htk_mode = false;
			bool is_librosa = false;
			std::string norm = "slaney";
			//// Amount of dithering, 0.0 means no dither.
			//float preemph_coeff = 0.97f;    // Preemphasis coefficient.
			//bool remove_dc_offset = true;   // Subtract mean of wave before FFT.
			//bool round_to_power_of_two = true;
			//float blackman_coeff = 0.42f;
			//// append an extra dimension with energy to the filter banks
			//bool use_energy = false;
			//// If true, compute log_energy before preemphasis and windowing
			//// If false, compute log_energy after preemphasis ans windowing
			//bool raw_energy = true;  // active iff use_energy==true
			//// if true (default), produce log-filterbank, else linear
			//bool use_log_fbank = true;
			//// if true (default), use power in filterbank
			//// analysis, else magnitude.
			//bool use_power = true;
			//// used only when is_librosa is true
			//bool use_slaney_mel_scale = true;
			//// used only when is_librosa is true
			//bool floor_to_int_bin = false;
		};

		typedef struct KnfOnlineFeature KnfOnlineFeature;

		LIBRARY_API FeatureOptions* GetFbankOptions(float dither, bool snip_edges, float sample_rate, int32_t num_bins, int32_t num_ceps, float frame_shift = 10.0f, float frame_length = 25.0f, float energy_floor = 0.0f, bool debug_mel = false, const char* window_type = "hamming", const char* feature_type = "fbank");
		LIBRARY_API KnfOnlineFeature* GetOnlineFbank(FeatureOptions* opts);
		LIBRARY_API void AcceptWaveform(KnfOnlineFeature* knfOnlineFeature, float sample_rate, float* samples, int samples_size);
		LIBRARY_API void InputFinished(KnfOnlineFeature* knfOnlineFeature);
		LIBRARY_API int32_t GetNumFramesReady(KnfOnlineFeature* knfOnlineFeature);
		LIBRARY_API void GetFbank(KnfOnlineFeature* knfOnlineFbank, int currFrameIndex, FbankData* /*out*/ pData);
		LIBRARY_API void GetFbanks(KnfOnlineFeature* knfOnlineFbank, int lastFrameIndex, FbankDatas* /*out*/ pData);
		std::vector<float> GetFrames(KnfOnlineFeature* knfOnlineFbank, int lastFrameIndex);
	}
#ifdef __cplusplus
}
#endif

#endif  // KALDI_NATIVE_FBANK_C_API_KNFWRAPPER_H_