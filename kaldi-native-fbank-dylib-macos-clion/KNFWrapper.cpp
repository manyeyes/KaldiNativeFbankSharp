//KNFWrapper.cpp
#include "KNFWrapper.h"

#include <iostream>
#include <mutex>  // NOLINT
#include "stdlib.h";
#include <cassert>


namespace knf
{
	std::mutex mutex_;

	struct KnfOnlineFbank {
		knf::OnlineFbank* impl;
	};

	FbankOptions* GetFbankOptions(float dither, bool snip_edges, float sample_rate, int32_t num_bins, float frame_shift, float frame_length, float energy_floor, bool debug_mel, const char* window_type)
	{
		FbankOptions* opts = new FbankOptions;
		opts->frame_opts.dither = 0;
		opts->frame_opts.snip_edges = snip_edges;
		opts->frame_opts.samp_freq = sample_rate;
		opts->frame_opts.window_type = window_type;
		opts->frame_opts.frame_shift_ms = frame_shift;
		opts->frame_opts.frame_length_ms = frame_length;
		opts->mel_opts.num_bins = 80;
		opts->mel_opts.debug_mel = debug_mel;
		opts->energy_floor = energy_floor;
		return opts;
	}

	KnfOnlineFbank* GetOnlineFbank(FbankOptions* opts)
	{
		KnfOnlineFbank* knfOnlineFbank = new KnfOnlineFbank;
		knfOnlineFbank->impl = new OnlineFbank(*opts);
		return knfOnlineFbank;
	}

	void AcceptWaveform(KnfOnlineFbank* knfOnlineFbank, float sample_rate, float* samples, int samples_size)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		std::vector<float> waveform{ samples, samples + samples_size };
		knfOnlineFbank->impl->AcceptWaveform(sample_rate, waveform.data(), waveform.size());
	}

	void  InputFinished(KnfOnlineFbank* knfOnlineFbank) {
		std::lock_guard<std::mutex> lock(mutex_);
		knfOnlineFbank->impl->InputFinished();
	}

	int32_t  GetNumFramesReady(KnfOnlineFbank* knfOnlineFbank) {
		std::lock_guard<std::mutex> lock(mutex_);
		int32_t n = knfOnlineFbank->impl->NumFramesReady();
		return n;
	}

	void GetFbank(KnfOnlineFbank* knfOnlineFbank, int currFrameIndex, FbankData* /*out*/ pData) {
		std::lock_guard<std::mutex> lock(mutex_);
		int32_t n = knfOnlineFbank->impl->NumFramesReady();
		assert(n > 0 && "Please first call AcceptWaveform()");
		//dis frame num, first is 0,second's next is 1
		int32_t discard_num = currFrameIndex == 0 ? 0 : 1;
		int32_t feature_dim = knfOnlineFbank->impl->Dim();
		//get frame at frame
		const float* f = knfOnlineFbank->impl->GetFrame(currFrameIndex);
		pData->data_length = feature_dim;
		pData->data = f;
		//pop the used frame
		knfOnlineFbank->impl->Pop(discard_num);
	}

	void GetFbanks(KnfOnlineFbank* knfOnlineFbank, int lastFrameIndex, FbankDatas* /*out*/ pData) {
		int32_t n = knfOnlineFbank->impl->NumFramesReady();
		assert(n > 0 && "Please first call AcceptWaveform()");
		std::vector<float> features = GetFrames(knfOnlineFbank, lastFrameIndex);
		pData->data = new float[features.size()];
		pData->data_length = features.size();
		for (int32_t i = 0; i != features.size(); ++i) {
			pData->data[i] = features[i];
		}
	}

	std::vector<float> GetFrames(KnfOnlineFbank* knfOnlineFbank, int lastFrameIndex) {
		std::lock_guard<std::mutex> lock(mutex_);
		int32_t n = knfOnlineFbank->impl->NumFramesReady();
		assert(n - lastFrameIndex >= 0 && "Please first call AcceptWaveform()");
		int32_t framesNum = lastFrameIndex == 0 ? n : n - (lastFrameIndex + 1);
		int32_t discard_num = lastFrameIndex == 0 ? n : n - (lastFrameIndex + 1);
		int32_t feature_dim = knfOnlineFbank->impl->Dim();
		std::vector<float> features(framesNum * feature_dim);
		float* p = features.data();
		int32_t currFrameIndex = lastFrameIndex == 0 ? 0 : lastFrameIndex + 1;
		for (int32_t i = currFrameIndex; i != n; ++i) {
			const float* f = knfOnlineFbank->impl->GetFrame(i);
			std::copy(f, f + feature_dim, p);
			p += feature_dim;
		}
		knfOnlineFbank->impl->Pop(discard_num);
		return features;
	}
}
