#include "pdt/core/anomaly.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <span>
#include <vector>

namespace pdt {

namespace {

std::vector<double> extract_values(std::span<const Sample> samples) {
    std::vector<double> values;
    values.reserve(samples.size());

    for (const auto& sample : samples) {
        values.push_back(sample.value);
    }

    return values;
}

Stats compute_stats_for_samples(std::span<const Sample> samples) {
    Stats stats{};

    if (samples.empty()) {
        return stats;
    }

    stats.count = samples.size();
    stats.min = samples.front().value;
    stats.max = samples.front().value;

    double sum = 0.0;
    for (const auto& sample : samples) {
        sum += sample.value;
        stats.min = std::min(stats.min, sample.value);
        stats.max = std::max(stats.max, sample.value);
    }

    stats.mean = sum / static_cast<double>(samples.size());

    double sq_sum = 0.0;
    for (const auto& sample : samples) {
        const double diff = sample.value - stats.mean;
        sq_sum += diff * diff;
    }

    stats.stddev = std::sqrt(sq_sum / static_cast<double>(samples.size()));
    return stats;
}

double percentile_sorted(const std::vector<double>& values, double p) {
    if (values.empty()) {
        return 0.0;
    }

    const double pos = p * static_cast<double>(values.size() - 1);
    const auto lo = static_cast<std::size_t>(std::floor(pos));
    const auto hi = static_cast<std::size_t>(std::ceil(pos));

    if (lo == hi) {
        return values[lo];
    }

    const double frac = pos - static_cast<double>(lo);
    return (values[lo] * (1.0 - frac)) + (values[hi] * frac);
}

void sort_and_trim(std::vector<Anomaly>& anomalies, std::size_t top_n) {
    std::ranges::sort(anomalies, [](const Anomaly& a, const Anomaly& b) {
        return std::abs(a.score) > std::abs(b.score);
    });

    if (anomalies.size() > top_n) {
        anomalies.resize(top_n);
    }
}

std::map<std::string, std::vector<Sample>> group_by_sensor(std::span<const Sample> samples) {
    std::map<std::string, std::vector<Sample>> groups;

    for (const auto& sample : samples) {
        groups[sample.sensor].push_back(sample);
    }

    return groups;
}

AnomalySummary make_summary(std::vector<Anomaly> anomalies, std::size_t top_n) {
    AnomalySummary summary{};
    summary.count = anomalies.size();

    sort_and_trim(anomalies, top_n);
    summary.top = std::move(anomalies);

    return summary;
}

AnomalySummary detect_zscore_for_samples(std::span<const Sample> samples, double threshold, std::size_t top_n) {
    if (samples.size() < 2 || threshold <= 0.0 || top_n == 0) {
        return {};
    }

    const auto stats = compute_stats_for_samples(samples);
    if (!(stats.stddev > 0.0)) {
        return {};
    }

    std::vector<Anomaly> anomalies;
    anomalies.reserve((samples.size() / 10) + 1);

    for (const auto& sample : samples) {
        const double z = (sample.value - stats.mean) / stats.stddev;

        if (std::abs(z) >= threshold) {
            anomalies.push_back(Anomaly{
                .timestamp = sample.timestamp,
                .sensor = sample.sensor,
                .value = sample.value,
                .score = z
            });
        }
    }

    return make_summary(std::move(anomalies), top_n);
}

AnomalySummary detect_iqr_for_samples(std::span<const Sample> samples, double threshold, std::size_t top_n) {
    if (samples.size() < 4 || threshold <= 0.0 || top_n == 0) {
        return {};
    }

    auto values = extract_values(samples);
    std::ranges::sort(values);

    const double q1 = percentile_sorted(values, 0.25);
    const double q3 = percentile_sorted(values, 0.75);
    const double iqr = q3 - q1;

    if (!(iqr > 0.0)) {
        return {};
    }

    const double lower = q1 - (threshold * iqr);
    const double upper = q3 + (threshold * iqr);

    std::vector<Anomaly> anomalies;

    for (const auto& sample : samples) {
        if (sample.value < lower) {
            anomalies.push_back(Anomaly{
                .timestamp = sample.timestamp,
                .sensor = sample.sensor,
                .value = sample.value,
                .score = (sample.value - lower) / iqr
            });
        } else if (sample.value > upper) {
            anomalies.push_back(Anomaly{
                .timestamp = sample.timestamp,
                .sensor = sample.sensor,
                .value = sample.value,
                .score = (sample.value - upper) / iqr
            });
        }
    }

    return make_summary(std::move(anomalies), top_n);
}

template <typename Detector>
AnomalySummary run_global_detector(std::span<const Sample> samples, double threshold, std::size_t top_n, Detector detector) {
    return detector(samples, threshold, top_n);
}

template <typename Detector>
std::map<std::string, AnomalySummary> run_per_sensor_detector(std::span<const Sample> samples, double threshold, std::size_t top_n, Detector detector) {
    std::map<std::string, AnomalySummary> result;
    const auto groups = group_by_sensor(samples);

    for (const auto& [sensor, sensor_samples] : groups) {
        result.emplace(sensor, detector(sensor_samples, threshold, top_n));
    }

    return result;
}

} // namespace

AnomalySummary detect_zscore_global(const DataSet& ds, double threshold, std::size_t top_n) {
    return run_global_detector(ds.samples(), threshold, top_n, detect_zscore_for_samples);
}

std::map<std::string, AnomalySummary> detect_zscore_per_sensor(const DataSet &ds, double threshold, std::size_t top_n) {
    return run_per_sensor_detector(ds.samples(), threshold, top_n, detect_zscore_for_samples);
}

AnomalySummary detect_iqr_global(const DataSet& ds, double threshold, std::size_t top_n) {
    return run_global_detector(ds.samples(), threshold, top_n, detect_iqr_for_samples);
}

std::map<std::string, AnomalySummary> detect_iqr_per_sensor(const DataSet& ds, double threshold, std::size_t top_n) {
    return run_per_sensor_detector(ds.samples(), threshold, top_n, detect_iqr_for_samples);
}

AnomalySummary detect_anomalies_global(const DataSet& ds, AnomalyMethod method, double threshold, std::size_t top_n) {
    using enum AnomalyMethod;

    switch (method) {
    case ZScore:
        return detect_zscore_global(ds, threshold, top_n);
    case IQR:
        return detect_iqr_global(ds, threshold, top_n);
    case MAD:
        return {};
    }

    return {};
}

std::map<std::string, AnomalySummary> detect_anomalies_per_sensor(const DataSet& ds, AnomalyMethod method, double threshold, std::size_t top_n) {
    using enum AnomalyMethod;

    switch (method) {
    case ZScore:
        return detect_zscore_per_sensor(ds, threshold, top_n);
    case IQR:
        return detect_iqr_per_sensor(ds, threshold, top_n);
    case MAD:
        return {};
    }

    return {};
}

} // namespace pdt
