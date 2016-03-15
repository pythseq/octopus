//
//  haplotype_liklihood_model.cpp
//  Octopus
//
//  Created by Daniel Cooke on 25/09/2015.
//  Copyright © 2015 Oxford University. All rights reserved.
//

#include "haplotype_liklihood_model.hpp"

#include <utility>
#include <cmath>
#include <limits>
#include <cassert>

#include "mappable.hpp"
#include "aligned_read.hpp"

#include <iostream> // TEST

namespace Octopus
{
    namespace
    {
        std::size_t num_out_of_range_bases(const std::size_t mapping_position,
                                           const AlignedRead& read, const Haplotype& haplotype)
        {
            const auto alignment_size = sequence_size(read) + mapping_position + 15;
            
            if (alignment_size > sequence_size(haplotype)) {
                return alignment_size - sequence_size(haplotype);
            }
            
            return 0;
        }
        
        bool is_in_range(const std::size_t mapping_position,
                         const AlignedRead& read, const Haplotype& haplotype)
        {
            return num_out_of_range_bases(mapping_position, read, haplotype) == 0;
        }
    } // namespace
    
    template <typename InputIt, typename T>
    double log_probability(const AlignedRead& read, const Haplotype& haplotype,
                           InputIt first_mapping_position, InputIt last_mapping_position,
                           const T& gap_open_penalities, const PairHMM::Model& model)
    {
        assert(contains(haplotype, read));
        
        const auto original_mapping_position = begin_distance(read, haplotype);
        
//                if (read.get_cigar_string() == parse_cigar_string("10S240M")
//                    && is_reference(haplotype)) {
//                    const auto it  = std::next(haplotype.get_sequence().cbegin(), original_mapping_position);
//                    const auto it2 = std::next(it, sequence_size(read));
//        
//                    std::cout << read.get_sequence() << std::endl;
//                    std::copy(it, it2, std::ostreambuf_iterator<char>(std::cout));
//        
//                    auto qitr = std::cbegin(read.get_qualities());
//        
//                    const auto d = std::inner_product(it, it2, std::cbegin(read.get_sequence()), 0,
//                                                      std::plus<void>(),
//                                                      std::not_equal_to<void>());
//        
//                    const auto c = std::inner_product(it, it2, std::cbegin(read.get_sequence()), 0,
//                                                      std::plus<void>(),
//                                                      [&] (const auto lhs, const auto rhs) -> int {
//                                                          if (lhs == rhs) {
//                                                              ++qitr;
//                                                              return 0;
//                                                          }
//                                                          //std::cout << (unsigned) *qitr << std::endl;
//                                                          return *qitr++;
//                                                      });
//        
//                    std::cout << d << " " << c << std::endl;
//        
//                    //exit(0);
//                }
        
        //        if (read.get_cigar_string() == parse_cigar_string("10S240M")
        //            && haplotype.contains(ContigAllele(ContigRegion(38082235, 38082236), "A"))
        //            && haplotype.contains(ContigAllele(ContigRegion(38082237, 38082238), "A"))
        //            && haplotype.contains(ContigAllele(ContigRegion(38082255, 38082256), "A"))
        //            && haplotype.contains(ContigAllele(ContigRegion(38082270, 38082271), "A"))) {
        //            const auto it  = std::next(haplotype.get_sequence().cbegin(), original_mapping_position);
        //            const auto it2 = std::next(it, sequence_size(read));
        //
        //            //std::copy(it, it2, std::ostreambuf_iterator<char>(std::cout));
        //
        //            auto qitr = std::cbegin(read.get_qualities());
        //
        //            const auto d = std::inner_product(it, it2, std::cbegin(read.get_sequence()), 0,
        //                                              std::plus<void>(),
        //                                              std::not_equal_to<void>());
        //
        //            const auto c = std::inner_product(it, it2, std::cbegin(read.get_sequence()), 0,
        //                                              std::plus<void>(),
        //                                              [&] (const auto lhs, const auto rhs) -> int {
        //                                                  if (lhs == rhs) {
        //                                                      ++qitr;
        //                                                      return 0;
        //                                                  }
        //                                                  return *qitr++;
        //                                              });
        //
        //            std::cout << d << " " << c << std::endl;
        //
        //            exit(0);
        //        }
        
        //                if (read.get_cigar_string() == parse_cigar_string("250M")
        //                    && contig_region(read) == ContigRegion(38082110, 38082360)
        //                    && haplotype.contains(ContigAllele(ContigRegion(38082235, 38082236), "A"))
        //                    && haplotype.contains(ContigAllele(ContigRegion(38082237, 38082238), "A"))
        //                    && haplotype.contains(ContigAllele(ContigRegion(38082255, 38082256), "A"))
        //                    && haplotype.contains(ContigAllele(ContigRegion(38082270, 38082271), "A"))) {
        //                    std::transform(read.get_qualities().cbegin(), read.get_qualities().cend(),
        //                                   std::ostream_iterator<char>(std::cout),
        //                                   [] (const auto q) { return q + 40; });
        //                    std::cout << std::endl;
        //
        //                    const auto it  = std::next(haplotype.get_sequence().cbegin(), original_mapping_position);
        //                    const auto it2 = std::next(it, sequence_size(read));
        //
        //                    //std::copy(it, it2, std::ostreambuf_iterator<char>(std::cout));
        //
        //                    auto qitr = std::cbegin(read.get_qualities());
        //
        //                    const auto d = std::inner_product(it, it2, std::cbegin(read.get_sequence()), 0,
        //                                                      std::plus<void>(),
        //                                                      std::not_equal_to<void>());
        //
        //                    const auto c = std::inner_product(it, it2, std::cbegin(read.get_sequence()), 0,
        //                                                      std::plus<void>(),
        //                                                      [&] (const auto lhs, const auto rhs) -> int {
        //                                                          if (lhs == rhs) {
        //                                                              ++qitr;
        //                                                              return 0;
        //                                                          }
        //                                                          return *qitr++;
        //                                                      });
        //                    
        //                    std::cout << d << " " << c << std::endl;
        //                    
        //                    exit(0);
        //                }
        
        auto max_log_probability = std::numeric_limits<double>::lowest();
        
        bool is_original_position_mapped {false};
        bool has_in_range_mapping_position {false};
        
        std::for_each(first_mapping_position, last_mapping_position,
                      [&] (const auto position) {
                          if (is_in_range(position, read, haplotype)) {
                              has_in_range_mapping_position = true;
                              
                              auto cur = PairHMM::align_around_offset(haplotype.get_sequence(), read.get_sequence(),
                                                                      read.get_qualities(), gap_open_penalities,
                                                                      position, model);
                              
                              if (cur > max_log_probability) {
                                  max_log_probability = cur;
                              }
                          }
                          
                          if (position == original_mapping_position) {
                              is_original_position_mapped = true;
                          }
                      });
        
        if (!is_original_position_mapped && is_in_range(original_mapping_position, read, haplotype)) {
            has_in_range_mapping_position = true;
            
            auto cur = PairHMM::align_around_offset(haplotype.get_sequence(), read.get_sequence(),
                                                    read.get_qualities(), gap_open_penalities,
                                                    original_mapping_position, model);
            
            if (cur > max_log_probability) {
                max_log_probability = cur;
            }
        }
        
        if (!has_in_range_mapping_position) {
            const auto min_shift = num_out_of_range_bases(original_mapping_position, read, haplotype);
            
            assert(original_mapping_position >= min_shift);
            
            auto final_mapping_position = original_mapping_position - min_shift;
            
            max_log_probability = PairHMM::align_around_offset(haplotype.get_sequence(), read.get_sequence(),
                                                               read.get_qualities(), gap_open_penalities,
                                                               final_mapping_position, model);
        }
        
        assert(max_log_probability > std::numeric_limits<double>::lowest());
        
        return max_log_probability;
    }
    
    // public methods
    
    HaplotypeLikelihoodModel::HaplotypeLikelihoodModel(const Haplotype& haplotype,
                                                       FlankState flank_state)
    :
    indel_error_model_ {},
    haplotype_ {haplotype},
    haplotype_gap_open_penalities_ {indel_error_model_.calculate_gap_open_penalties(haplotype)},
    haplotype_flank_state_ {flank_state},
    model_ {2, 3}
    {
        if (haplotype_flank_state_.has_lhs_flank_inactive_candidates) {
            model_.lhs_flank_size = begin_distance(contig_region(haplotype_.get()),
                                                   haplotype_flank_state_.active_region);
        }
        
        if (haplotype_flank_state_.has_rhs_flank_inactive_candidates) {
            model_.rhs_flank_size = end_distance(contig_region(haplotype_.get()),
                                                 haplotype_flank_state_.active_region);
        }
    }
    
    double HaplotypeLikelihoodModel::log_probability(const AlignedRead& read,
                                                     MapPositionItr first_mapping_position,
                                                     MapPositionItr last_mapping_position) const
    {
        return Octopus::log_probability(read, haplotype_,
                                        first_mapping_position, last_mapping_position,
                                        haplotype_gap_open_penalities_, model_);
    }
    
} // namespace Octopus
