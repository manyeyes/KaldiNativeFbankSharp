//KNFWrapper.cpp
#include "pch.h"
#include "KNFWrapper.h"

#include <iostream>
#include <mutex>  // NOLINT
#include "stdlib.h";
#include <cassert>


namespace knf
{
	/*int32_t last_frame_index_ = 0;
	int32_t last_frame_num_ = 0;*/
	std::mutex mutex_;

	struct KnfOnlineFeature {
		knf::IOnlineFeature* impl;
	};

	FeatureOptions* GetFbankOptions(float dither, bool snip_edges, float sample_rate, int32_t num_bins, int32_t num_ceps, float frame_shift, float frame_length, float energy_floor, bool debug_mel, const char* window_type, const char* feature_type)
	{
		FeatureOptions* opts = new FeatureOptions;
		opts->dither = dither;
		opts->snip_edges = snip_edges;
		opts->sample_rate = sample_rate;
		opts->window_type = window_type;
		opts->frame_shift = frame_shift;
		opts->frame_length = frame_length;
		opts->num_bins = num_bins;
		opts->num_ceps = num_ceps;
		opts->debug_mel = debug_mel;
		opts->energy_floor = energy_floor;
		opts->feature_type = feature_type;
		return opts;
	}

	KnfOnlineFeature* GetOnlineFbank(FeatureOptions* opts)
	{
		KnfOnlineFeature* knfOnlineFeature = new KnfOnlineFeature;
		if (opts->feature_type == "fbank") {
			FbankOptions* opts_ = new FbankOptions;
			opts_->frame_opts.dither = opts->dither;
			opts_->frame_opts.snip_edges = opts->snip_edges;
			opts_->frame_opts.samp_freq = opts->sample_rate;
			opts_->frame_opts.window_type = opts->window_type;
			opts_->frame_opts.frame_shift_ms = opts->frame_shift;
			opts_->frame_opts.frame_length_ms = opts->frame_length;
			opts_->mel_opts.num_bins = opts->num_bins;
			opts_->mel_opts.debug_mel = opts->debug_mel;
			opts_->energy_floor = opts->energy_floor;
			knfOnlineFeature->impl = new knf::OnlineFbankAdapter(*opts_);
		}
		if (opts->feature_type == "mfcc") {
			MfccOptions* opts_ = new MfccOptions;
			opts_->frame_opts.dither = opts->dither;//eg. 0
			opts_->num_ceps = opts->num_ceps;//eg. 40
			opts_->mel_opts.num_bins = opts->num_bins;//eg. 40
			opts_->mel_opts.high_freq = -200;//eg. -200
			opts_->frame_opts.snip_edges = opts->snip_edges;//eg. false
			knfOnlineFeature->impl = new knf::OnlineMfccAdapter(*opts_);
		}
		if (opts->feature_type == "whisper") {
			WhisperFeatureOptions* opts_ = new WhisperFeatureOptions;
			opts_->dim = opts->num_bins;
			knfOnlineFeature->impl = new knf::OnlineWhisperFbankAdapter(*opts_);
		}
		return knfOnlineFeature;

	}

	void AcceptWaveform(KnfOnlineFeature* knfOnlineFeature, float sample_rate, float* samples, int samples_size)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		std::vector<float> waveform{ samples, samples + samples_size };
		knfOnlineFeature->impl->AcceptWaveform(sample_rate, waveform.data(), waveform.size());
	}

	void  InputFinished(KnfOnlineFeature* knfOnlineFeature) {
		std::lock_guard<std::mutex> lock(mutex_);
		knfOnlineFeature->impl->InputFinished();
	}

	int32_t  GetNumFramesReady(KnfOnlineFeature* knfOnlineFeature) {
		std::lock_guard<std::mutex> lock(mutex_);
		int32_t n = knfOnlineFeature->impl->NumFramesReady();
		return n;
	}

	void GetFbank(KnfOnlineFeature* knfOnlineFeature, int currFrameIndex, FbankData* /*out*/ pData) {
		std::lock_guard<std::mutex> lock(mutex_);
		int32_t n = knfOnlineFeature->impl->NumFramesReady();
		assert(n > 0 && "Please first call AcceptWaveform()");
		//dis frame num, first is 0,second's next is 1
		int32_t discard_num = currFrameIndex == 0 ? 0 : 1;
		int32_t feature_dim = knfOnlineFeature->impl->Dim();
		//get frame at frame
		const float* f = knfOnlineFeature->impl->GetFrame(currFrameIndex);
		pData->data_length = feature_dim;
		pData->data = f;
		//pop the used frame
		knfOnlineFeature->impl->Pop(discard_num);
	}

	//void GetFbank(KnfOnlineFeature* knfOnlineFeature, int frame, FbankData* /*out*/ pData) {
	//	int32_t n = knfOnlineFeature->impl->NumFramesReady();
	//	assert(n > 0 && "Please first call AcceptWaveform()");
	//	n = 1;
	//	int32_t discard_num = last_frame_num_;
	//	int32_t feature_dim = knfOnlineFeature->impl->Dim();
	//	knfOnlineFeature->impl->Pop(discard_num);

	//	int32_t i = frame == last_frame_index_ ? frame : last_frame_index_;
	//	const float* f = knfOnlineFeature->impl->GetFrame(i);
	//	pData->data_length = feature_dim;
	//	pData->data = f;

	//	last_frame_index_ = last_frame_index_ + n;
	//	last_frame_num_ = 1;
	//}

	void GetFbanks(KnfOnlineFeature* knfOnlineFeature, int lastFrameIndex, FbankDatas* /*out*/ pData) {
		int32_t n = knfOnlineFeature->impl->NumFramesReady();
		assert(n > 0 && "Please first call AcceptWaveform()");
		std::vector<float> features = GetFrames(knfOnlineFeature, lastFrameIndex);
		pData->data = new float[features.size()];
		pData->data_length = features.size();
		for (int32_t i = 0; i != features.size(); ++i) {
			pData->data[i] = features[i];
		}
	}

	std::vector<float> GetFrames(KnfOnlineFeature* knfOnlineFeature, int lastFrameIndex) {
		std::lock_guard<std::mutex> lock(mutex_);
		int32_t n = knfOnlineFeature->impl->NumFramesReady();
		assert(n - lastFrameIndex >= 0 && "Please first call AcceptWaveform()");		
		/*int32_t n = framesNum - last_frame_index_;
		assert(n > 0 && "Please first call AcceptWaveform()");*/
		int32_t framesNum = lastFrameIndex == 0 ? n : n - (lastFrameIndex + 1);
		int32_t discard_num = lastFrameIndex == 0 ? n : n-(lastFrameIndex + 1);
		int32_t feature_dim = knfOnlineFeature->impl->Dim();		
		std::vector<float> features(framesNum * feature_dim);
		float* p = features.data();
		int32_t currFrameIndex = lastFrameIndex == 0 ? 0 : lastFrameIndex + 1;
		for (int32_t i = currFrameIndex; i != n; ++i) {
			const float* f = knfOnlineFeature->impl->GetFrame(i);
			std::copy(f, f + feature_dim, p);
			p += feature_dim;
		}
		knfOnlineFeature->impl->Pop(discard_num);
		/*last_frame_index_ = last_frame_index_ + n;
		last_frame_num_ = n;*/
		return features;
	}

	//void GetFbanks(KnfOnlineFeature* knfOnlineFeature, int framesNum, FbankDatas* /*out*/ pData) {
	//	int32_t n = knfOnlineFeature->impl->NumFramesReady();
	//	assert(n > 0 && "Please first call AcceptWaveform()");
	//	std::vector<float> features = GetFrames(knfOnlineFeature, framesNum);
	//	pData->data = new float[features.size()];
	//	pData->data_length = features.size();
	//	for (int32_t i = 0; i != features.size(); ++i) {
	//		pData->data[i] = features[i];
	//	}
	//}

	//std::vector<float> GetFrames(KnfOnlineFeature* knfOnlineFeature, int framesNum) {
	//	std::lock_guard<std::mutex> lock(mutex_);
	//	int32_t n = framesNum - last_frame_index_;
	//	assert(n > 0 && "Please first call AcceptWaveform()");
	//	int32_t discard_num = last_frame_num_;
	//	int32_t feature_dim = knfOnlineFeature->impl->Dim();
	//	knfOnlineFeature->impl->Pop(discard_num);
	//	std::vector<float> features(n * feature_dim);
	//	float* p = features.data();
	//	for (int32_t i = 0; i != n; ++i) {
	//		const float* f = knfOnlineFeature->impl->GetFrame(i + last_frame_index_);
	//		std::copy(f, f + feature_dim, p);
	//		p += feature_dim;
	//	}
	//	last_frame_index_ = last_frame_index_ + n;
	//	last_frame_num_ = n;
	//	return features;
	//}

}
