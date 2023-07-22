//KNFWrapper.cpp
#include "pch.h"
#include "KNFWrapper.h"

#include <iostream>
#include <mutex>  // NOLINT
#include "stdlib.h";
#include <cassert>


namespace knf
{
	int32_t last_frame_index_ = 0;
	int32_t last_frame_num_ = 0;
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
		last_frame_index_ = 0;
		last_frame_num_ = 0;
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

	void GetFbank(KnfOnlineFbank* knfOnlineFbank, int frame, FbankData* /*out*/ pData) {
		int32_t n = knfOnlineFbank->impl->NumFramesReady();
		//int32_t n = framesNum - last_frame_index_;
		assert(n > 0 && "Please first call AcceptWaveform()");
		n = 1;
		int32_t discard_num = last_frame_num_;
		int32_t feature_dim = knfOnlineFbank->impl->Dim();
		knfOnlineFbank->impl->Pop(discard_num);

		int32_t i = frame == last_frame_index_ ? frame : last_frame_index_;
		const float* f = knfOnlineFbank->impl->GetFrame(i);
		pData->data_length = feature_dim;
		pData->data = f;

		last_frame_index_ = last_frame_index_ + n;
		last_frame_num_ = 1;
	}

	void GetFbanks(KnfOnlineFbank* knfOnlineFbank, int framesNum, FbankDatas* /*out*/ pData) {
		int32_t n = knfOnlineFbank->impl->NumFramesReady();
		assert(n > 0 && "Please first call AcceptWaveform()");
		std::vector<float> features = GetFrames(knfOnlineFbank, framesNum);
		pData->data = new float[features.size()];
		pData->data_length = features.size();
		for (int32_t i = 0; i != features.size(); ++i) {
			pData->data[i] = features[i];
		}
	}

	std::vector<float> GetFrames(KnfOnlineFbank* knfOnlineFbank, int framesNum) {
		std::lock_guard<std::mutex> lock(mutex_);
		int32_t n = framesNum - last_frame_index_;
		assert(n > 0 && "Please first call AcceptWaveform()");
		int32_t discard_num = last_frame_num_;
		int32_t feature_dim = knfOnlineFbank->impl->Dim();
		knfOnlineFbank->impl->Pop(discard_num);
		std::vector<float> features(n * feature_dim);
		float* p = features.data();
		for (int32_t i = 0; i != n; ++i) {
			const float* f = knfOnlineFbank->impl->GetFrame(i+ last_frame_index_);
			std::copy(f, f + feature_dim, p);
			p += feature_dim;
		}
		last_frame_index_ = last_frame_index_+ n;
		last_frame_num_ = n;
		return features;
	}

}
