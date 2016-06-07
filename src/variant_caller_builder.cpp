//
//  variant_caller_builder.cpp
//  Octopus
//
//  Created by Daniel Cooke on 11/01/2016.
//  Copyright © 2016 Oxford University. All rights reserved.
//

#include "variant_caller_builder.hpp"

#include "individual_caller.hpp"
#include "population_caller.hpp"
#include "cancer_caller.hpp"
#include "pedigree_caller.hpp"
#include "phaser.hpp"

namespace Octopus
{
// public methods

VariantCallerBuilder::Parameters::Parameters(const ReferenceGenome& reference,
                                             const ReadPipe& read_pipe,
                                             const CandidateGeneratorBuilder& candidate_generator_builder,
                                             HaplotypeGenerator::Builder haplotype_generator_builder)
:
reference {reference},
read_pipe {read_pipe},
candidate_generator_builder {candidate_generator_builder},
haplotype_generator_builder {std::move(haplotype_generator_builder)}
{}

VariantCallerBuilder::VariantCallerBuilder(const ReferenceGenome& reference,
                                           const ReadPipe& read_pipe,
                                           const CandidateGeneratorBuilder& candidate_generator_builder,
                                           HaplotypeGenerator::Builder haplotype_generator_builder)
:
parameters_ {reference, read_pipe, candidate_generator_builder, std::move(haplotype_generator_builder)},
factory_ {generate_factory()}
{}

VariantCallerBuilder::VariantCallerBuilder(const VariantCallerBuilder& other)
:
parameters_ {other.parameters_},
factory_    {generate_factory()}
{}

VariantCallerBuilder& VariantCallerBuilder::operator=(const VariantCallerBuilder& other)
{
    parameters_ = other.parameters_;
    factory_    = generate_factory();
    return *this;
}

VariantCallerBuilder::VariantCallerBuilder(VariantCallerBuilder&& other)
:
parameters_ {std::move(other.parameters_)},
factory_    {generate_factory()}
{}

VariantCallerBuilder& VariantCallerBuilder::operator=(VariantCallerBuilder&& other)
{
    std::swap(parameters_, other.parameters_);
    factory_ = generate_factory();
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_reference(const ReferenceGenome& reference) noexcept
{
    parameters_.reference = reference;
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_read_pipe(const ReadPipe& read_pipe) noexcept
{
    parameters_.read_pipe = read_pipe;
    return *this;
}

VariantCallerBuilder&
VariantCallerBuilder::set_candidate_generator_builder(const CandidateGeneratorBuilder& generator) noexcept
{
    parameters_.candidate_generator_builder = generator;
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_ploidy(const unsigned ploidy) noexcept
{
    parameters_.ploidy = ploidy;
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_caller(std::string caller)
{
    parameters_.caller = std::move(caller);
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_refcall_type(const VariantCaller::RefCallType refcall_type) noexcept
{
    parameters_.refcall_type = refcall_type;
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_sites_only() noexcept
{
    parameters_.call_sites_only = true;
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_min_variant_posterior(const double min_posterior) noexcept
{
    parameters_.min_variant_posterior = min_posterior;
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_min_refcall_posterior(const double min_posterior) noexcept
{
    parameters_.min_refcall_posterior = min_posterior;
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_max_haplotypes(const unsigned max_haplotypes) noexcept
{
    parameters_.max_haplotypes = max_haplotypes;
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_min_haplotype_posterior(const double p) noexcept
{
    parameters_.min_haplotype_posterior = p;
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_flank_scoring(const bool allow_flank_scoring) noexcept
{
    parameters_.allow_flank_scoring = allow_flank_scoring;
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_model_filtering(bool b) noexcept
{
    parameters_.allow_model_filtering = b;
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_min_phase_score(const double min_phase_score) noexcept
{
    parameters_.min_phase_score = min_phase_score;
    return *this;
}

// cancer
VariantCallerBuilder& VariantCallerBuilder::set_normal_sample(SampleIdType normal_sample)
{
    parameters_.normal_sample = std::move(normal_sample);
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_somatic_mutation_rate(double somatic_mutation_rate)
{
    parameters_.somatic_mutation_rate = somatic_mutation_rate;
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_min_somatic_posterior(const double min_posterior) noexcept
{
    parameters_.min_somatic_posterior = min_posterior;
    return *this;
}

// trio

VariantCallerBuilder& VariantCallerBuilder::set_maternal_sample(SampleIdType mother)
{
    parameters_.maternal_sample = std::move(mother);
    return *this;
}

VariantCallerBuilder& VariantCallerBuilder::set_paternal_sample(SampleIdType father)
{
    parameters_.paternal_sample = std::move(father);
    return *this;
}

// pedigree

VariantCallerBuilder& VariantCallerBuilder::set_pedigree(Pedigree pedigree)
{
    parameters_.pedigree = std::move(pedigree);
    return *this;
}

// build

std::unique_ptr<VariantCaller> VariantCallerBuilder::build() const
{
    if (factory_.count(parameters_.caller) == 0) return nullptr;
    return factory_.at(parameters_.caller)();
}

// private methods

VariantCallerBuilder::CallerFactoryMap VariantCallerBuilder::generate_factory() const
{
    VariantCaller::CallerParameters general_parameters {
        parameters_.refcall_type,
        parameters_.call_sites_only,
        parameters_.max_haplotypes,
        parameters_.min_haplotype_posterior,
        parameters_.allow_flank_scoring,
        parameters_.allow_model_filtering
    };
    
    return CallerFactoryMap {
        {"individual", [this, general_parameters = std::move(general_parameters)] () {
            return std::make_unique<IndividualVariantCaller>(VariantCaller::CallerComponents {
                                                                 parameters_.reference,
                                                                 parameters_.read_pipe,
                                                                 parameters_.candidate_generator_builder.get().build(),
                                                                 parameters_.haplotype_generator_builder,
                                                                 Phaser {parameters_.min_phase_score}
                                                             },
                                                             std::move(general_parameters),
                                                             IndividualVariantCaller::CallerParameters {
                                                                 parameters_.min_variant_posterior,
                                                                 parameters_.min_refcall_posterior,
                                                                 parameters_.ploidy
                                                             });
        }},
        {"population", [this, general_parameters = std::move(general_parameters)] () {
            return std::make_unique<PopulationVariantCaller>(VariantCaller::CallerComponents {
                                                                 parameters_.reference,
                                                                 parameters_.read_pipe,
                                                                 parameters_.candidate_generator_builder.get().build(),
                                                                 parameters_.haplotype_generator_builder,
                                                                 Phaser {parameters_.min_phase_score}
                                                             },
                                                             std::move(general_parameters),
                                                             PopulationVariantCaller::CallerParameters {
                                                                 parameters_.min_variant_posterior,
                                                                 parameters_.min_refcall_posterior,
                                                                 parameters_.ploidy
                                                             });
        }},
        {"cancer", [this, general_parameters = std::move(general_parameters)] () {
            return std::make_unique<CancerVariantCaller>(VariantCaller::CallerComponents {
                                                             parameters_.reference,
                                                             parameters_.read_pipe,
                                                             parameters_.candidate_generator_builder.get().build(),
                                                             parameters_.haplotype_generator_builder,
                                                             Phaser {parameters_.min_phase_score}
                                                         },
                                                         std::move(general_parameters),
                                                         CancerVariantCaller::CallerParameters {
                                                             parameters_.min_variant_posterior,
                                                             parameters_.min_somatic_posterior,
                                                             parameters_.min_refcall_posterior,
                                                             parameters_.ploidy,
                                                             parameters_.normal_sample,
                                                             parameters_.somatic_mutation_rate,
                                                             50'000
                                                         });
        }}
    };
}
} // namespace Octopus
