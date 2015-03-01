//
//  aligned_read.h
//  Octopus
//
//  Created by Daniel Cooke on 11/02/2015.
//  Copyright (c) 2015 Oxford University. All rights reserved.
//

#ifndef __Octopus__aligned_read__
#define __Octopus__aligned_read__

#include <string>
#include <cstdint>
#include <ostream>
#include <algorithm> // std::transform

#include "genomic_region.h"
#include "cigar_string.h"
#include "comparable.h"

class AlignedRead : Comparable<AlignedRead>
{
public:
    using SizeType    = std::uint_fast32_t;
    using QualityType = std::uint_fast8_t;
    using Qualities   = std::vector<QualityType>;
    
    AlignedRead() = delete;
    template <typename GenomicRegion_, typename String_, typename Qualities_, typename CigarString_>
    explicit AlignedRead(GenomicRegion_&& reference_region, String_&& sequence,
                         Qualities_&& qualities, CigarString_&& cigar_string,
                         SizeType insert_size, std::string mate_contig_name,
                         SizeType mate_begin, QualityType mapping_quality);
    
    AlignedRead(const AlignedRead&)            = default;
    AlignedRead& operator=(const AlignedRead&) = default;
    AlignedRead(AlignedRead&&)                 = default;
    AlignedRead& operator=(AlignedRead&&)      = default;
    
    const GenomicRegion& get_region() const;
    const std::string& get_contig_name() const;
    SizeType get_begin() const noexcept;
    SizeType get_end() const noexcept;
    const std::string& get_sequence() const;
    const Qualities& get_qualities() const;
    QualityType get_mapping_quality() const;
    SizeType get_sequence_size() const;
    const CigarString& get_cigar_string() const;
    SizeType get_insert_size() const;
    const std::string& get_mate_contig_name() const;
    SizeType get_mate_begin() const;
    
    template <typename CompressionAlgorithm> void compress(const CompressionAlgorithm& c);
    template <typename CompressionAlgorithm> void decompress(const CompressionAlgorithm& c);
    
private:
    GenomicRegion reference_region_;
    std::string mate_contig_name_;
    std::string sequence_;
    CigarString cigar_string_;
    Qualities qualities_;
    SizeType insert_size_;
    SizeType mate_begin_;
    QualityType mapping_quality_;
};

template <typename GenomicRegion_, typename String_, typename Qualities_, typename CigarString_>
inline AlignedRead::AlignedRead(GenomicRegion_&& reference_region, String_&& sequence,
                                Qualities_&& qualities, CigarString_&& cigar_string,
                                SizeType insert_size, std::string mate_contig_name,
                                SizeType mate_begin, QualityType mapping_quality)
:reference_region_ {std::forward<GenomicRegion_>(reference_region)},
 sequence_ {std::forward<String_>(sequence)},
 qualities_ {std::forward<Qualities_>(qualities)},
 cigar_string_ {std::forward<CigarString_>(cigar_string)},
 insert_size_ {insert_size},
 mate_contig_name_ {mate_contig_name},
 mate_begin_ {mate_begin},
 mapping_quality_ {mapping_quality}
{}

inline const GenomicRegion& AlignedRead::get_region() const
{
    return reference_region_;
}

inline const std::string& AlignedRead::get_contig_name() const
{
    return reference_region_.get_contig_name();
}

inline AlignedRead::SizeType AlignedRead::get_begin() const noexcept
{
    return reference_region_.get_begin();
}

inline AlignedRead::SizeType AlignedRead::get_end() const noexcept
{
    return reference_region_.get_end();
}

inline const std::string& AlignedRead::get_sequence() const
{
    return sequence_;
}

inline const AlignedRead::Qualities& AlignedRead::get_qualities() const
{
    return qualities_;
}

inline AlignedRead::QualityType AlignedRead::get_mapping_quality() const
{
    return mapping_quality_;
}

inline AlignedRead::SizeType AlignedRead::get_sequence_size() const
{
    return static_cast<SizeType>(sequence_.size());
}

inline const CigarString& AlignedRead::get_cigar_string() const
{
    return cigar_string_;
}

inline AlignedRead::SizeType AlignedRead::get_insert_size() const
{
    return insert_size_;
}

inline const std::string& AlignedRead::get_mate_contig_name() const
{
    return mate_contig_name_;
}

inline AlignedRead::SizeType AlignedRead::get_mate_begin() const
{
    return mate_begin_;
}

template <typename CompressionAlgorithm>
void AlignedRead::compress(const CompressionAlgorithm& c)
{
    sequence_ = CompressionAlgorithm::compress(sequence_);
}

template <typename CompressionAlgorithm>
void AlignedRead::decompress(const CompressionAlgorithm& c)
{
    sequence_ = CompressionAlgorithm::decompress(sequence_);
}

inline bool operator==(const AlignedRead& lhs, const AlignedRead& rhs)
{
    // The order of these comparisons should ensure optimal lazy evaluation
    return lhs.get_mapping_quality() == rhs.get_mapping_quality() &&
            lhs.get_region() == rhs.get_region() &&
            lhs.get_cigar_string() == rhs.get_cigar_string();
}

inline bool operator< (const AlignedRead& lhs, const AlignedRead& rhs)
{
    return lhs.get_begin() < rhs.get_begin();
}

namespace std {
    template <> struct hash<AlignedRead>
    {
        size_t operator()(const AlignedRead& r) const
        {
            return hash<GenomicRegion>()(r.get_region()); // TODO: improve this hash
        }
    };
}

inline std::ostream& operator<<(std::ostream& os, const AlignedRead::Qualities& qualities)
{
    std::transform(std::cbegin(qualities), std::cend(qualities),
                   std::ostream_iterator<AlignedRead::QualityType>(os, ""),
                   [] (auto q) { return q + 33; });
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const AlignedRead& a_read)
{
    os << a_read.get_region() << '\n';
    os << a_read.get_sequence() << '\n';
    os << a_read.get_qualities() << '\n';
    os << a_read.get_cigar_string() << '\n';
    os << static_cast<unsigned>(a_read.get_mapping_quality()) << '\n';
    os << a_read.get_insert_size() << '\n';
    os << a_read.get_mate_contig_name() << '\n';
    os << a_read.get_mate_begin();
    return os;
}

#endif /* defined(__Octopus__aligned_read__) */