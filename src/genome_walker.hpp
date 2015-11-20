//
//  genome_walker.hpp
//  Octopus
//
//  Created by Daniel Cooke on 17/10/2015.
//  Copyright © 2015 Oxford University. All rights reserved.
//

#ifndef genome_walker_hpp
#define genome_walker_hpp

#include "common.hpp"
#include "mappable_set.hpp"

class GenomicRegion;
class Variant;

namespace Octopus {

class GenomeWalker
{
public:
    using ContigNameType = GenomicRegion::StringType;
    using Candidates     = MappableSet<Variant>;
    
    enum class IndicatorLimit { SharedWithPreviousRegion, NoLimit };
    enum class ExtensionLimit { WithinReadLengthOfFirstIncluded, SharedWithFrontier, NoLimit };
    enum class ExpansionLimit { UpToExcluded, WithinReadLength, UpToExcludedWithinReadLength,
                                NoExpansion };
    
    GenomeWalker() = delete;
    explicit GenomeWalker(unsigned max_indicators, unsigned max_included,
                          IndicatorLimit indicator_limit, ExtensionLimit extension_limit,
                          ExpansionLimit expansion_limit = ExpansionLimit::UpToExcludedWithinReadLength);
    ~GenomeWalker() = default;
    
    GenomeWalker(const GenomeWalker&)            = default;
    GenomeWalker& operator=(const GenomeWalker&) = default;
    GenomeWalker(GenomeWalker&&)                 = default;
    GenomeWalker& operator=(GenomeWalker&&)      = default;
    
    GenomicRegion walk(const ContigNameType& contig, const ReadMap& reads,
                       const Candidates& candidates);
    
    GenomicRegion walk(const GenomicRegion& previous_region, const ReadMap& reads,
                       const Candidates& candidates);
    
private:
    const unsigned max_indicators_;
    const unsigned max_included_;
    const IndicatorLimit indicator_limit_;
    const ExtensionLimit extension_limit_;
    const ExpansionLimit expansion_limit_;
};

} // namespace Octopus

#endif /* genome_walker_hpp */