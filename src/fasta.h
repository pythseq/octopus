//
//  fasta.h
//  Octopus
//
//  Created by Daniel Cooke on 10/02/2015.
//  Copyright (c) 2015 Oxford University. All rights reserved.
//

#ifndef __Octopus__fasta__
#define __Octopus__fasta__

#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <unordered_map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include "reference_genome_impl.h"
#include "bioio.h"

class GenomicRegion;

namespace fs = boost::filesystem;

class Fasta : public IReferenceGenomeImpl
{
public:
    using SequenceType = std::string;
    using SizeType    = IReferenceGenomeImpl::SizeType;
    
    Fasta() = delete;
    Fasta(std::string fasta_path);
    Fasta(std::string fasta_path, std::string fasta_index_path);
    ~Fasta() noexcept override = default;
    
    Fasta(const Fasta&)            = default;
    Fasta& operator=(const Fasta&) = default;
    Fasta(Fasta&&)                 = default;
    Fasta& operator=(Fasta&&)      = default;
    
    std::string get_reference_name() override;
    std::vector<std::string> get_contig_names() override;
    SizeType get_contig_size(std::string contig_name) override;
    SequenceType get_sequence(const GenomicRegion& a_region) override;

private:
    fs::path fasta_path_;
    fs::path fasta_index_path_;
    std::ifstream fasta_;
    std::unordered_map<std::string, bioio::FastaIndex> fasta_contig_indices_;
    
    bool is_valid_fasta() const;
};

#endif /* defined(__Octopus__fasta__) */