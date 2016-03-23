//
//  haplotype_filter.cpp
//  Octopus
//
//  Created by Daniel Cooke on 02/03/2016.
//  Copyright © 2016 Oxford University. All rights reserved.
//

#include "haplotype_filter.hpp"

#include <unordered_map>
#include <deque>
#include <algorithm>
#include <iterator>
#include <limits>

#include "haplotype.hpp"
#include "haplotype_likelihood_cache.hpp"
#include "maths.hpp"

namespace Octopus
{
    double max_read_likelihood(const Haplotype& haplotype,
                               const std::vector<SampleIdType>& samples,
                               const HaplotypeLikelihoodCache& haplotype_likelihoods)
    {
        auto result = std::numeric_limits<double>::lowest();
        
        for (const auto& sample : samples) {
            for (const double likelihood : haplotype_likelihoods.log_likelihoods(sample, haplotype)) {
                if (likelihood > result) result = likelihood;
                if (Maths::almost_zero(likelihood)) break;
            }
        }
        
        return result;
    }
    
    void
    filter_by_likelihood_sum(const std::vector<SampleIdType>& samples,
                             std::vector<Haplotype>& haplotypes,
                             const std::vector<Haplotype>::iterator first_removable,
                             const HaplotypeLikelihoodCache& haplotype_likelihoods,
                             const std::size_t n,
                             std::vector<Haplotype>& removed)
    {
        std::cout << "here = n" << std::endl;
    }
    
    std::vector<Haplotype>
    filter_by_maximum_likelihood(std::vector<Haplotype>& haplotypes,
                                 const std::vector<SampleIdType>& samples,
                                 const HaplotypeLikelihoodCache& haplotype_likelihoods,
                                 const std::size_t n)
    {
        std::vector<Haplotype> result {};
        
        if (haplotypes.size() <= n) {
            std::sort(std::begin(haplotypes), std::end(haplotypes));
            return result;
        }
        
        std::unordered_map<Haplotype, double> max_liklihoods {haplotypes.size()};
        
        for (const auto& haplotype : haplotypes) {
            max_liklihoods.emplace(haplotype, max_read_likelihood(haplotype, samples, haplotype_likelihoods));
        }
        
        const auto nth = std::next(std::begin(haplotypes), n);
        
        std::nth_element(std::begin(haplotypes), nth, std::end(haplotypes),
                         [&] (const auto& lhs, const auto& rhs) {
                             return max_liklihoods.at(lhs) > max_liklihoods.at(rhs);
                         });
        
//        const auto nth_max_liklihood = max_liklihoods.at(*nth);
//        
//        const auto it = std::find_if(std::make_reverse_iterator(std::prev(nth)),
//                                     std::make_reverse_iterator(std::begin(haplotypes)),
//                                     [nth_max_liklihood, &max_liklihoods] (const Haplotype& haplotype) {
//                                         return max_liklihoods.at(haplotype) == nth_max_liklihood;
//                                     });
//        
//        if (it != std::make_reverse_iterator(std::begin(haplotypes))) {
//            std::sort(std::begin(haplotypes), std::end(haplotypes),
//                             [&] (const auto& lhs, const auto& rhs) {
//                                 return max_liklihoods.at(lhs) > max_liklihoods.at(rhs);
//                             });
//            
//            const auto er = std::equal_range(std::begin(haplotypes), std::end(haplotypes),
//                                             nth_max_liklihood,
//                                             [&] (const auto& lhs, const auto& rhs) {
//                                                 return max_liklihoods.at(lhs) > max_liklihoods.at(rhs);
//                                             });
//            
//            const auto num_removable = std::distance(er.second, std::end(haplotypes));
//            result.reserve(num_removable);
//            std::move(er.second, std::end(haplotypes), std::begin(result));
//            haplotypes.erase(er.second, std::end(haplotypes));
//            
//            // now deal with haplotypes in er
//            
//            filter_by_likelihood_sum(samples, haplotypes, er.first, haplotype_likelihoods,
//                                     n - num_removable, result);
//            
//            return result;
//        }
        
        std::sort(std::begin(haplotypes), nth);
        std::sort(nth, std::end(haplotypes));
        
        std::deque<Haplotype> duplicates {};
        
        std::set_intersection(std::begin(haplotypes), nth, nth, std::end(haplotypes),
                              std::back_inserter(duplicates));
        
        result.assign(std::make_move_iterator(nth), std::make_move_iterator(std::end(haplotypes)));
        
        haplotypes.erase(nth, std::end(haplotypes));
        
        for (const auto& duplicate : duplicates) {
            const auto er = std::equal_range(std::begin(haplotypes), std::end(haplotypes), duplicate);
            haplotypes.erase(er.first, er.second);
        }
        
        return result;
    }
    
    std::vector<Haplotype>
    filter_n_haplotypes(std::vector<Haplotype>& haplotypes, const std::vector<SampleIdType>& samples,
                        const HaplotypeLikelihoodCache& haplotype_likelihoods, const std::size_t n)
    {
        auto result = filter_by_maximum_likelihood(haplotypes, samples, haplotype_likelihoods, n);
        
        // TODO: try some other filters if this one fails
        
        return result;
    }
} // namespace Octopus