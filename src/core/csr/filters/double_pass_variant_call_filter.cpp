// Copyright (c) 2015-2018 Daniel Cooke
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.

#include "double_pass_variant_call_filter.hpp"

#include <utility>
#include <iterator>
#include <algorithm>
#include <cassert>

#include <boost/range/combine.hpp>

#include "io/variant/vcf_reader.hpp"
#include "io/variant/vcf_writer.hpp"
#include "utils/append.hpp"

namespace octopus { namespace csr {

DoublePassVariantCallFilter::DoublePassVariantCallFilter(FacetFactory facet_factory,
                                                         std::vector<MeasureWrapper> measures,
                                                         OutputOptions output_config,
                                                         ConcurrencyPolicy threading,
                                                         boost::optional<ProgressMeter&> progress)
: VariantCallFilter {std::move(facet_factory), std::move(measures), std::move(output_config), threading}
, info_log_ {logging::InfoLogger {}}
, progress_ {progress}
, current_contig_ {}
{}

void DoublePassVariantCallFilter::filter(const VcfReader& source, VcfWriter& dest, const SampleList& samples) const
{
    assert(dest.is_header_written());
    make_registration_pass(source, samples);
    prepare_for_classification(info_log_);
    make_filter_pass(source, samples, dest);
}

void DoublePassVariantCallFilter::log_registration_pass(Log& log) const
{
    log << "CSR: Starting registration pass";
}

void DoublePassVariantCallFilter::make_registration_pass(const VcfReader& source, const SampleList& samples) const
{
    if (info_log_) log_registration_pass(*info_log_);
    prepare_for_registration(samples);
    if (progress_) progress_->start();
    std::size_t record_idx {0};
    if (can_measure_multiple_blocks()) {
        for (auto p = source.iterate(); p.first != p.second;) {
            const auto blocks = read_next_blocks(p.first, p.second, samples);
            record(blocks, record_idx, samples);
            for (const auto& block : blocks) record_idx += block.size();
        }
    } else if (can_measure_single_call()) {
        auto p = source.iterate();
        std::for_each(std::move(p.first), std::move(p.second), [&] (const VcfRecord& call) { record(call, record_idx++, samples); });
    } else {
        for (auto p = source.iterate(); p.first != p.second;) {
            const auto block = read_next_block(p.first, p.second, samples);
            record(block, record_idx, samples);
            record_idx += block.size();
        }
    }
    if (progress_) progress_->stop();
}

void DoublePassVariantCallFilter::record(const VcfRecord& call, const std::size_t record_idx, const SampleList& samples) const
{
    record(call, measure(call), record_idx, samples);
}

void DoublePassVariantCallFilter::record(const CallBlock& block, const std::size_t record_idx, const SampleList& samples) const
{
    record(block, measure(block), record_idx, samples);
}

void DoublePassVariantCallFilter::record(const std::vector<CallBlock>& blocks, std::size_t record_idx, const SampleList& samples) const
{
    const auto measures = measure(blocks);
    assert(measures.size() == blocks.size());
    for (auto tup : boost::combine(blocks, measures)) {
        const auto& block = tup.get<0>();
        record(block, tup.get<1>(), record_idx, samples);
        record_idx += block.size();
    }
}

void DoublePassVariantCallFilter::record(const VcfRecord& call, const MeasureVector& measures,
                                         const std::size_t record_idx, const SampleList& samples) const
{
    for (std::size_t sample_idx {0}; sample_idx < samples.size(); ++sample_idx) {
        this->record(record_idx, sample_idx, get_sample_values(measures, measures_, sample_idx));
    }
    log_progress(mapped_region(call));
}

void DoublePassVariantCallFilter::record(const CallBlock& block, const MeasureBlock& measures,
                                         std::size_t record_idx, const SampleList& samples) const
{
    assert(measures.size() == block.size());
    for (auto tup : boost::combine(block, measures)) {
        record(tup.get<0>(), tup.get<1>(), record_idx++, samples);
    }
}

void DoublePassVariantCallFilter::log_filter_pass_start(Log& log) const
{
    log << "CSR: Starting filtering pass";
}

void DoublePassVariantCallFilter::make_filter_pass(const VcfReader& source, const SampleList& samples, VcfWriter& dest) const
{
    if (info_log_) log_filter_pass_start(*info_log_);
    if (progress_) {
        progress_->reset();
        progress_->set_max_tick_size(10);
        progress_->start();
    }
    auto p = source.iterate();
    std::size_t idx {0};
    std::for_each(std::move(p.first), std::move(p.second), [&] (const VcfRecord& call) { filter(call, idx++, samples, dest); });
    if (progress_) progress_->stop();
}

std::vector<VariantCallFilter::Classification>
DoublePassVariantCallFilter::classify(std::size_t call_idx, const SampleList& samples) const
{
    std::vector<Classification> result(samples.size());
    for (std::size_t sample_idx {0}; sample_idx < samples.size(); ++sample_idx) {
        result[sample_idx] = this->classify(call_idx, sample_idx);
    }
    return result;
}

void DoublePassVariantCallFilter::filter(const VcfRecord& call, const std::size_t call_idx, const SampleList& samples,
                                         VcfWriter& dest) const
{
    const auto sample_classifications = classify(call_idx, samples);
    const auto call_classification = merge(sample_classifications);
    write(call, call_classification, samples, sample_classifications, dest);
    log_progress(mapped_region(call));
}

static auto expand_lhs_to_zero(const GenomicRegion& region)
{
    return GenomicRegion {region.contig_name(), 0, region.end()};
}

void DoublePassVariantCallFilter::log_progress(const GenomicRegion& region) const
{
    if (progress_) {
        if (current_contig_) {
            if (*current_contig_ != region.contig_name()) {
                progress_->log_completed(*current_contig_);
                current_contig_ = region.contig_name();
            }
        } else {
            current_contig_ = region.contig_name();
        }
        progress_->log_completed(expand_lhs_to_zero(region));
    }
}

} // namespace csr
} // namespace octopus
