//
//  read_reader.hpp
//  Octopus
//
//  Created by Daniel Cooke on 11/02/2015.
//  Copyright (c) 2015 Oxford University. All rights reserved.
//

#ifndef __Octopus__read_reader__
#define __Octopus__read_reader__

#include <vector>
#include <cstddef>
#include <memory>
#include <mutex>
#include <utility>

#include <boost/filesystem/path.hpp>

#include "read_reader_impl.hpp"
#include "equitable.hpp"

class GenomicRegion;
class AlignedRead;

/*
 ReadReader provides a RAII threadsafe wrapper around a IReadReaderImpl
 */
class ReadReader : public Equitable<ReadReader>
{
public:
    using Path = boost::filesystem::path;
    
    using SampleIdType  = IReadReaderImpl::SampleIdType;
    using SizeType      = IReadReaderImpl::SizeType;
    using ReadContainer = IReadReaderImpl::ReadContainer;
    using SampleReadMap = IReadReaderImpl::SampleReadMap;
    using CoveragePair  = IReadReaderImpl::CoveragePair;
    
    ReadReader() = default;
    
    explicit ReadReader(const Path& file_path);
    
    ~ReadReader() = default;
    
    ReadReader(const ReadReader&)            = delete;
    ReadReader& operator=(const ReadReader&) = delete;
    ReadReader(ReadReader&&);
    ReadReader& operator=(ReadReader&&)      = delete;
    
    friend void swap(ReadReader& lhs, ReadReader& rhs) noexcept;
    
    bool is_open() const noexcept;
    void open();
    void close();
    
    const Path& path() const noexcept;
    
    std::vector<std::string> extract_reference_contig_names();
    unsigned count_reference_contigs();
    
    std::vector<SampleIdType> extract_samples();
    std::vector<std::string> extract_read_groups_in_sample(const SampleIdType& sample);
    
    std::vector<GenomicRegion> extract_possible_regions_in_file();
    
    bool has_contig_reads(const GenomicRegion::ContigNameType& contig);
    
    std::size_t count_reads(const GenomicRegion& region);
    std::size_t count_reads(const SampleIdType& sample, const GenomicRegion& region);
    
    CoveragePair find_covered_subregion(const GenomicRegion& region, std::size_t max_coverage);
    CoveragePair find_covered_subregion(const SampleIdType& sample, const GenomicRegion& region,
                                        std::size_t max_coverage);
    CoveragePair find_covered_subregion(const std::vector<SampleIdType>& samples, const GenomicRegion& region,
                                        std::size_t max_coverage);
    
    SampleReadMap fetch_reads(const GenomicRegion& region);
    ReadContainer fetch_reads(const SampleIdType& sample, const GenomicRegion& region);
    SampleReadMap fetch_reads(const std::vector<SampleIdType>& samples, const GenomicRegion& region);
    
private:
    Path file_path_;
    std::unique_ptr<IReadReaderImpl> the_impl_;
    mutable std::mutex mutex_;
};

bool operator==(const ReadReader& lhs, const ReadReader& rhs);

namespace std {
    template <> struct hash<ReadReader>
    {
        size_t operator()(const ReadReader& reader) const
        {
            return hash<string>()(reader.path().string());
        }
    };
}

namespace std
{
    template <> struct hash<reference_wrapper<const ReadReader>>
    {
        size_t operator()(reference_wrapper<const ReadReader> reader) const
        {
            return hash<ReadReader>()(reader);
        }
    };
} // namespace std

#endif /* defined(__Octopus__read_reader__) */
