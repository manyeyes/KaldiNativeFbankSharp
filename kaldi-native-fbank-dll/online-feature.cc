/**
 * Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)
 *
 * See LICENSE for clarification regarding multiple authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 // The content in this file is copied/modified from
 // This file is copied/modified from kaldi/src/feat/online-feature.cc

#include "pch.h"
#include "online-feature.h"

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

#include "feature-window.h"
#include "log.h"

namespace knf {

	RecyclingVector::RecyclingVector(int32_t items_to_hold)
		: items_to_hold_(items_to_hold == 0 ? -1 : items_to_hold),
		first_available_index_(0) {
	}

	const float* RecyclingVector::At(int32_t index) const {
		if (index < first_available_index_) {
			KNF_LOG(FATAL) << "Attempted to retrieve feature vector that was "
				"already removed by the RecyclingVector (index = "
				<< index << "; "
				<< "first_available_index = " << first_available_index_
				<< "; "
				<< "size = " << Size() << ")";
		}
		// 'at' does size checking.
		return items_.at(index - first_available_index_).data();
	}

	void RecyclingVector::PushBack(std::vector<float> item) {
		// Note: -1 is a larger number when treated as unsigned
		if (items_.size() == static_cast<size_t>(items_to_hold_)) {
			items_.pop_front();
			++first_available_index_;
		}
		items_.push_back(std::move(item));
	}

	int32_t RecyclingVector::Size() const {
		return first_available_index_ + static_cast<int32_t>(items_.size());
	}

	// discard the first n frames
	void RecyclingVector::Pop(int32_t n) {
		for (int32_t i = 0; i < n && !items_.empty(); ++i) {
			items_.pop_front();
			++first_available_index_;
		}
	}

	template <class C>
	OnlineGenericBaseFeature<C>::OnlineGenericBaseFeature(
		const typename C::Options& opts)
		: computer_(opts),
		window_function_(computer_.GetFrameOptions()),
		input_finished_(false),
		waveform_offset_(0) {
	}

	template <class C>
	void OnlineGenericBaseFeature<C>::AcceptWaveform(float sampling_rate,
		const float* waveform,
		int32_t n) {
		if (n == 0) {
			return;  // Nothing to do.
		}

		if (input_finished_) {
			KNF_LOG(FATAL) << "AcceptWaveform called after InputFinished() was called.";
		}

		KNF_CHECK_EQ(sampling_rate, computer_.GetFrameOptions().samp_freq);

		waveform_remainder_.insert(waveform_remainder_.end(), waveform, waveform + n);

		ComputeFeatures();
	}

	template <class C>
	void OnlineGenericBaseFeature<C>::InputFinished() {
		input_finished_ = true;
		ComputeFeatures();
	}

	template <class C>
	void OnlineGenericBaseFeature<C>::ComputeFeatures() {
		const FrameExtractionOptions& frame_opts = computer_.GetFrameOptions();

		int64_t num_samples_total = waveform_offset_ + waveform_remainder_.size();

		int32_t num_frames_old = features_.Size();

		int32_t num_frames_new =
			NumFrames(num_samples_total, frame_opts, input_finished_);

		KNF_CHECK_GE(num_frames_new, num_frames_old);

		// note: this online feature-extraction code does not support VTLN.
		float vtln_warp = 1.0;

		std::vector<float> window;
		bool need_raw_log_energy = computer_.NeedRawLogEnergy();

		for (int32_t frame = num_frames_old; frame < num_frames_new; ++frame) {
			std::fill(window.begin(), window.end(), 0);
			float raw_log_energy = 0.0;
			ExtractWindow(waveform_offset_, waveform_remainder_, frame, frame_opts,
				window_function_, &window,
				need_raw_log_energy ? &raw_log_energy : nullptr);

			std::vector<float> this_feature(computer_.Dim());

			computer_.Compute(raw_log_energy, vtln_warp, &window, this_feature.data());
			features_.PushBack(std::move(this_feature));
		}

		// OK, we will now discard any portion of the signal that will not be
		// necessary to compute frames in the future.
		int64_t first_sample_of_next_frame =
			FirstSampleOfFrame(num_frames_new, frame_opts);

		int32_t samples_to_discard = first_sample_of_next_frame - waveform_offset_;

		if (samples_to_discard > 0) {
			// discard the leftmost part of the waveform that we no longer need.
			int32_t new_num_samples =
				static_cast<int32_t>(waveform_remainder_.size()) - samples_to_discard;

			if (new_num_samples <= 0) {
				// odd, but we'll try to handle it.
				waveform_offset_ += waveform_remainder_.size();
				waveform_remainder_.resize(0);
			}
			else {
				std::vector<float> new_remainder(new_num_samples);

				std::copy(waveform_remainder_.begin() + samples_to_discard,
					waveform_remainder_.end(), new_remainder.begin());
				waveform_offset_ += samples_to_discard;

				waveform_remainder_.swap(new_remainder);
			}
		}
	}

	template class OnlineGenericBaseFeature<FbankComputer>;
	template class OnlineGenericBaseFeature<MfccComputer>;
	template class OnlineGenericBaseFeature<WhisperFeatureComputer>;

	// 适配器类实现
	OnlineFbankAdapter::OnlineFbankAdapter(const FbankComputer::Options& opts) : impl_(opts) {}

	void OnlineFbankAdapter::AcceptWaveform(float sampling_rate, const float* waveform, int32_t n) {
		impl_.AcceptWaveform(sampling_rate, waveform, n);
	}

	void OnlineFbankAdapter::InputFinished() {
		impl_.InputFinished();
	}

	void OnlineFbankAdapter::Pop(int32_t n) {
		impl_.Pop(n);
	}

	int32_t OnlineFbankAdapter::NumFramesReady() const {
		return impl_.NumFramesReady();
	}

	const float* OnlineFbankAdapter::GetFrame(int32_t frame) const {
		return impl_.GetFrame(frame);
	}

	int32_t OnlineFbankAdapter::Dim() const {
		return impl_.Dim();
	}

	// OnlineMfccAdapter 实现
	OnlineMfccAdapter::OnlineMfccAdapter(const MfccComputer::Options& opts) : impl_(opts) {}

	void OnlineMfccAdapter::AcceptWaveform(float sampling_rate, const float* waveform, int32_t n) {
		impl_.AcceptWaveform(sampling_rate, waveform, n);
	}

	void OnlineMfccAdapter::InputFinished() {
		impl_.InputFinished();
	}

	void OnlineMfccAdapter::Pop(int32_t n) {
		impl_.Pop(n);
	}

	int32_t OnlineMfccAdapter::NumFramesReady() const {
		return impl_.NumFramesReady();
	}

	const float* OnlineMfccAdapter::GetFrame(int32_t frame) const {
		return impl_.GetFrame(frame);
	}

	int32_t OnlineMfccAdapter::Dim() const {
		return impl_.Dim();
	}

	// OnlineWhisperFbankAdapter 实现
	OnlineWhisperFbankAdapter::OnlineWhisperFbankAdapter(const WhisperFeatureComputer::Options& opts)
		: impl_(opts) {
	}

	void OnlineWhisperFbankAdapter::AcceptWaveform(float sampling_rate, const float* waveform, int32_t n) {
		impl_.AcceptWaveform(sampling_rate, waveform, n);
	}

	void OnlineWhisperFbankAdapter::InputFinished() {
		impl_.InputFinished();
	}

	void OnlineWhisperFbankAdapter::Pop(int32_t n) {
		impl_.Pop(n);
	}

	int32_t OnlineWhisperFbankAdapter::NumFramesReady() const {
		return impl_.NumFramesReady();
	}

	const float* OnlineWhisperFbankAdapter::GetFrame(int32_t frame) const {
		return impl_.GetFrame(frame);
	}

	int32_t OnlineWhisperFbankAdapter::Dim() const {
		return impl_.Dim();
	}

}  // namespace knf
