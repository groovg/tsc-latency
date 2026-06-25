#pragma once

#include <bit>
#include <cstdint>
#include <vector>

namespace tsclat {

// Log-linear histogram: values below 2^sub_bits are counted exactly, and each
// octave above that is split into 2^sub_bits linear sub-buckets, giving a bounded
// relative error (~1/2^sub_bits) across the whole u64 range in a few KB.
class Histogram {
public:
    explicit Histogram(int sub_bits = 6)
        : sub_bits_(sub_bits),
          mask_((std::uint64_t{1} << sub_bits) - 1),
          counts_(index_for(UINT64_MAX) + 1, 0) {}

    void record(std::uint64_t value) {
        ++counts_[index_for(value)];
        ++count_;
        sum_ += static_cast<double>(value);
        if (value > max_) {
            max_ = value;
        }
    }

    void record_corrected(std::uint64_t value, std::uint64_t expected_interval) {
        record(value);
        if (expected_interval == 0 || value <= expected_interval) {
            return;
        }
        for (std::uint64_t missing = value - expected_interval; missing >= expected_interval;
             missing -= expected_interval) {
            record(missing);
        }
    }

    std::uint64_t count() const { return count_; }
    std::uint64_t max() const { return max_; }
    double mean() const { return count_ == 0 ? 0.0 : sum_ / static_cast<double>(count_); }

    std::uint64_t value_at_quantile(double quantile) const {
        if (count_ == 0) {
            return 0;
        }
        std::uint64_t rank = static_cast<std::uint64_t>(quantile * static_cast<double>(count_) + 0.5);
        if (rank == 0) {
            rank = 1;
        }
        std::uint64_t cumulative = 0;
        for (std::size_t i = 0; i < counts_.size(); ++i) {
            cumulative += counts_[i];
            if (cumulative >= rank) {
                return value_from_index(i);
            }
        }
        return max_;
    }

private:
    std::size_t index_for(std::uint64_t value) const {
        if (value < (std::uint64_t{1} << sub_bits_)) {
            return static_cast<std::size_t>(value);
        }
        const int leading = 63 - std::countl_zero(value);
        const int shift = leading - sub_bits_;
        const std::uint64_t sub = (value >> shift) & mask_;
        return static_cast<std::size_t>(
            (static_cast<std::uint64_t>(leading - sub_bits_ + 1) << sub_bits_) + sub);
    }

    std::uint64_t value_from_index(std::size_t index) const {
        if (index < (std::uint64_t{1} << sub_bits_)) {
            return index;
        }
        const std::uint64_t octave = static_cast<std::uint64_t>(index) >> sub_bits_;
        const std::uint64_t sub = static_cast<std::uint64_t>(index) & mask_;
        const int leading = static_cast<int>(octave) + sub_bits_ - 1;
        return ((std::uint64_t{1} << sub_bits_) | sub) << (leading - sub_bits_);
    }

    int sub_bits_;
    std::uint64_t mask_;
    std::vector<std::uint64_t> counts_;
    std::uint64_t count_ = 0;
    std::uint64_t max_ = 0;
    double sum_ = 0.0;
};

}  // namespace tsclat
