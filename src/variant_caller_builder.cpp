//
//  variant_caller_builder.cpp
//  Octopus
//
//  Created by Daniel Cooke on 11/01/2016.
//  Copyright © 2016 Oxford University. All rights reserved.
//

#include "variant_caller_builder.hpp"

#include "population_caller.hpp"
#include "cancer_caller.hpp"
#include "pedigree_caller.hpp"

#include <iostream> // DEBUG

namespace Octopus
{
    // public methods
    
    VariantCallerBuilder::VariantCallerBuilder(const ReferenceGenome& reference,
                                               ReadPipe& read_pipe,
                                               const CandidateGeneratorBuilder& candidate_generator_builder)
    :
    reference_ {reference},
    read_pipe_ {read_pipe},
    candidate_generator_builder_ {candidate_generator_builder},
    factory_ {generate_factory()}
    {}
    
    VariantCallerBuilder::VariantCallerBuilder(const VariantCallerBuilder& other)
    :
    reference_                   {other.reference_},
    read_pipe_                   {other.read_pipe_},
    ploidy_                      {other.ploidy_},
    model_                       {other.model_},
    candidate_generator_builder_ {other.candidate_generator_builder_},
    refcall_type_                {other.refcall_type_},
    min_variant_posterior_       {other.min_variant_posterior_},
    min_refcall_posterior_       {other.min_refcall_posterior_},
    normal_sample_               {other.normal_sample_},
    min_somatic_posterior_       {other.min_somatic_posterior_},
    call_somatics_only_          {other.call_somatics_only_},
    maternal_sample_             {other.maternal_sample_},
    paternal_sample_             {other.paternal_sample_},
    pedigree_                    {other.pedigree_},
    factory_                     {generate_factory()}
    {}
    
    VariantCallerBuilder& VariantCallerBuilder::operator=(const VariantCallerBuilder& other)
    {
        reference_                   = other.reference_;
        read_pipe_                   = other.read_pipe_;
        ploidy_                      = other.ploidy_;
        model_                       = other.model_;
        candidate_generator_builder_ = other.candidate_generator_builder_;
        refcall_type_                = other.refcall_type_;
        min_variant_posterior_       = other.min_variant_posterior_;
        min_refcall_posterior_       = other.min_refcall_posterior_;
        normal_sample_               = other.normal_sample_;
        min_somatic_posterior_       = other.min_somatic_posterior_;
        call_somatics_only_          = other.call_somatics_only_;
        maternal_sample_             = other.maternal_sample_;
        paternal_sample_             = other.paternal_sample_;
        pedigree_                    = other.pedigree_;
        factory_                   = generate_factory();
        return *this;
    }
    
    VariantCallerBuilder::VariantCallerBuilder(VariantCallerBuilder&& other)
    :
    reference_                   {std::move(other.reference_)},
    read_pipe_                   {std::move(other.read_pipe_)},
    ploidy_                      {std::move(other.ploidy_)},
    model_                       {std::move(other.model_)},
    candidate_generator_builder_ {std::move(other.candidate_generator_builder_)},
    refcall_type_                {std::move(other.refcall_type_)},
    min_variant_posterior_       {std::move(other.min_variant_posterior_)},
    min_refcall_posterior_       {std::move(other.min_refcall_posterior_)},
    normal_sample_               {std::move(other.normal_sample_)},
    min_somatic_posterior_       {std::move(other.min_somatic_posterior_)},
    call_somatics_only_          {std::move(other.call_somatics_only_)},
    maternal_sample_             {std::move(other.maternal_sample_)},
    paternal_sample_             {std::move(other.paternal_sample_)},
    pedigree_                    {std::move(other.pedigree_)},
    factory_                     {generate_factory()}
    {}
    
    VariantCallerBuilder& VariantCallerBuilder::operator=(VariantCallerBuilder&& other)
    {
        using std::swap;
        swap(reference_                  , other.reference_);
        swap(read_pipe_                  , other.read_pipe_);
        swap(ploidy_                     , other.ploidy_);
        swap(model_                      , other.model_);
        swap(candidate_generator_builder_, other.candidate_generator_builder_);
        swap(refcall_type_               , other.refcall_type_);
        swap(min_variant_posterior_      , other.min_variant_posterior_);
        swap(min_refcall_posterior_      , other.min_refcall_posterior_);
        swap(normal_sample_              , other.normal_sample_);
        swap(min_somatic_posterior_      , other.min_somatic_posterior_);
        swap(call_somatics_only_         , other.call_somatics_only_);
        swap(maternal_sample_            , other.maternal_sample_);
        swap(paternal_sample_            , other.paternal_sample_);
        swap(pedigree_                   , other.pedigree_);
        factory_ = generate_factory();
        return *this;
    }
    
    void VariantCallerBuilder::set_reference(const ReferenceGenome& reference) noexcept
    {
        reference_ = reference;
    }
    
    void VariantCallerBuilder::set_read_pipe(ReadPipe& read_pipe) noexcept
    {
        read_pipe_ = read_pipe;
    }
    
    void VariantCallerBuilder::set_candidate_generator_builder(const CandidateGeneratorBuilder& candidate_generator_builder) noexcept
    {
        candidate_generator_builder_ = candidate_generator_builder;
    }
    
    void VariantCallerBuilder::set_ploidy(const unsigned ploidy) noexcept
    {
        ploidy_ = ploidy;
    }
    
    void VariantCallerBuilder::set_model(std::string model)
    {
        model_ = std::move(model);
    }
    
    void VariantCallerBuilder::set_refcall_type(const VariantCaller::RefCallType refcall_type) noexcept
    {
        refcall_type_ = refcall_type;
    }
    
    void VariantCallerBuilder::set_min_variant_posterior(const double min_posterior) noexcept
    {
        min_variant_posterior_ = min_posterior;
    }
    
    void VariantCallerBuilder::set_min_refcall_posterior(const double min_posterior) noexcept
    {
        min_refcall_posterior_ = min_posterior;
    }
    
    // cancer
    void VariantCallerBuilder::set_normal_sample(SampleIdType normal_sample)
    {
        normal_sample_ = std::move(normal_sample);
    }
    
    void VariantCallerBuilder::set_min_somatic_posterior(const double min_posterior) noexcept
    {
        min_somatic_posterior_ = min_posterior;
    }
    
    void VariantCallerBuilder::set_somatic_only_calls() noexcept
    {
        call_somatics_only_ = true;
    }
    
    void VariantCallerBuilder::set_somatic_and_variant_calls() noexcept
    {
        call_somatics_only_ = false;
    }
    
    void VariantCallerBuilder::set_somatic_and_variant_and_refcalls_calls() noexcept
    {
        call_somatics_only_ = false;
    }
    
    // trio
    
    void VariantCallerBuilder::set_maternal_sample(SampleIdType mother)
    {
        maternal_sample_ = std::move(mother);
    }
    
    void VariantCallerBuilder::set_paternal_sample(SampleIdType father)
    {
        paternal_sample_ = std::move(father);
    }
    
    // pedigree
    
    void VariantCallerBuilder::set_pedigree(Pedigree pedigree)
    {
        pedigree_ = std::move(pedigree);
    }
    
    // build
    
    std::unique_ptr<VariantCaller> VariantCallerBuilder::build() const
    {
        if (factory_.count(model_) == 0) return nullptr;
        return factory_.at(model_)();
    }
    
    // private methods
    
    VariantCallerBuilder::ModelFactoryMap VariantCallerBuilder::generate_factory() const
    {
        return ModelFactoryMap {
            {"population", [this] () {
                return std::make_unique<PopulationVariantCaller>(reference_,
                                                                 read_pipe_,
                                                                 candidate_generator_builder_.get().build(),
                                                                 refcall_type_,
                                                                 min_variant_posterior_,
                                                                 min_refcall_posterior_,
                                                                 ploidy_);
            }},
            {"cancer", [this] () {
                return std::make_unique<CancerVariantCaller>(reference_,
                                                             read_pipe_,
                                                             candidate_generator_builder_.get().build(),
                                                             refcall_type_,
                                                             min_variant_posterior_,
                                                             min_somatic_posterior_,
                                                             min_refcall_posterior_,
                                                             normal_sample_.get(),
                                                             call_somatics_only_);
            }},
            {"trio", [this] () {
                return std::make_unique<PedigreeVariantCaller>(reference_,
                                                               read_pipe_,
                                                               candidate_generator_builder_.get().build(),
                                                               ploidy_,
                                                               maternal_sample_.get(),
                                                               paternal_sample_.get(),
                                                               min_variant_posterior_);
            }}
        };
    }
} // namespace Octopus