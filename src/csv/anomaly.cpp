#include "pdt/csv/anomaly.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <span>
#include <utility>
#include <vector>

namespace pdt {
namespace {

double median_sorted(const std::vector<double>& values) {
    // Expects values sorted in ascending order.
    // For even-sized input returns the arithmetic mean of the two middle values.
    if (values.empty()) {  return 0.0; }

    const std::size_t n = values.size();
    const std::size_t mid = n / 2;

    if ((n % 2) == 0) {
        return (values[mid - 1] + values[mid]) / 2.0;
    }

    return values[mid];
}

AnomalySummary make_summary(std::vector<Anomaly> anomalies, std::size_t top_n) {
    AnomalySummary summary{};
    summary.all = std::move(anomalies);
    summary.top = select_top_anomalies(summary.all, top_n);
    return summary;
}

AnomalySummary detect_zscore_for_samples(std::span<const Sample> samples, double threshold, std::size_t top_n) {
    if (samples.size() < 2 || threshold <= 0.0 || top_n == 0) { return {}; }

    const auto stats = compute_stats(samples);
    if (!(stats.stddev > 0.0)) { return {}; }

    std::vector<Anomaly> anomalies;
    anomalies.reserve((samples.size() / 10) + 1); // assuming number of anomalies 10 % of number of samples

    for (std::size_t i = 0; i < samples.size(); ++i) {
        const auto& sample = samples[i];
        const double z = (sample.value - stats.mean) / stats.stddev;

        if (std::abs(z) >= threshold) {
            anomalies.push_back(Anomaly{.timestamp = sample.timestamp,
                                        .sensor = sample.sensor,
                                        .value = sample.value,
                                        .score = z,
                                        .index = i
            });
        }
    }

    return make_summary(std::move(anomalies), top_n);
}

AnomalySummary detect_iqr_for_samples(std::span<const Sample> samples, double threshold, std::size_t top_n) {
    // IQR rule:
    // lower = Q1 - threshold * IQR
    // upper = Q3 + threshold * IQR
    // score expresses how far the value is beyond the violated bound, normalized by IQR.
    if (samples.size() < 4 || threshold <= 0.0 || top_n == 0) { return {}; }

    const auto stats = compute_stats(samples);

    const double q1  = stats.q1;
    const double q3  = stats.q3;
    const double iqr = q3 - q1;

    if (!(iqr > 0.0)) { return {}; }

    const double lower = q1 - (threshold * iqr);
    const double upper = q3 + (threshold * iqr);

    std::vector<Anomaly> anomalies;

    for (std::size_t i = 0; i < samples.size(); ++i) {
        const auto& sample = samples[i];

        if (sample.value < lower) {
            anomalies.push_back(Anomaly{.timestamp = sample.timestamp,
                                        .sensor = sample.sensor,
                                        .value = sample.value,
                                        .score = (sample.value - lower) / iqr,
                                        .index = i
            });
        } else if (sample.value > upper) {
            anomalies.push_back(Anomaly{.timestamp = sample.timestamp,
                                        .sensor = sample.sensor,
                                        .value = sample.value,
                                        .score = (sample.value - upper) / iqr,
                                        .index = i
            });
        }
    }

    return make_summary(std::move(anomalies), top_n);
}

AnomalySummary detect_mad_for_samples(std::span<const Sample> samples, double threshold, std::size_t top_n) {
    // MAD-based detection:
    // 1. compute the median of values
    // 2. compute absolute deviations from that median
    // 3. MAD is the median of those absolute deviations
    // score = (value - median) / MAD
    //
    // Note: if MAD == 0, the data has no robust spread for this method,
    // so anomaly detection is skipped.
    if (samples.size() < 3 || threshold <= 0.0 || top_n == 0) {
        return {};
    }

    const auto stats = compute_stats(samples);
    const double median = stats.median;

    std::vector<double> abs_deviations;
    abs_deviations.reserve(samples.size());

    for (const auto& sample : samples) {
        abs_deviations.push_back(std::abs(sample.value - median));
    }

    std::ranges::sort(abs_deviations);
    const double mad = median_sorted(abs_deviations);

    if (!(mad > 0.0)) { return {}; }

    std::vector<Anomaly> anomalies;
    anomalies.reserve((samples.size() / 10) + 1);

    for (std::size_t i = 0; i < samples.size(); ++i) {
        const auto& sample = samples[i];
        const double score = (sample.value - median) / mad;

        if (std::abs(score) >= threshold) {
            anomalies.push_back(Anomaly{.timestamp = sample.timestamp,
                                        .sensor = sample.sensor,
                                        .value = sample.value,
                                        .score = score,
                                        .index = i
            });
        }
    }

    return make_summary(std::move(anomalies), top_n);
}

template <typename Detector>
std::map<std::string, AnomalySummary> run_per_sensor_detector(const DataSet& dataSet, double threshold, std::size_t top_n, Detector detector) {
    // Applies the same detector independently to samples grouped by sensor.
    std::map<std::string, AnomalySummary> result;

    for (const auto& [sensor, sensorDataSet] : dataSet.split_by_sensor()) {
        result.emplace(sensor, detector(sensorDataSet.samples(), threshold, top_n));
    }

    return result;
}

AnomalySummary detect_zscore_global(const DataSet& ds, double threshold, std::size_t top_n) {
    return detect_zscore_for_samples(ds.samples(), threshold, top_n);
}

std::map<std::string, AnomalySummary> detect_zscore_per_sensor(const DataSet &ds, double threshold, std::size_t top_n) {
    return run_per_sensor_detector(ds, threshold, top_n, detect_zscore_for_samples);
}

AnomalySummary detect_iqr_global(const DataSet& ds, double threshold, std::size_t top_n) {
    return detect_iqr_for_samples(ds.samples(), threshold, top_n);
}

std::map<std::string, AnomalySummary> detect_iqr_per_sensor(const DataSet& ds, double threshold, std::size_t top_n) {
    return run_per_sensor_detector(ds, threshold, top_n, detect_iqr_for_samples);
}

AnomalySummary detect_mad_global(const DataSet& ds, double threshold, std::size_t top_n) {
    return detect_mad_for_samples(ds.samples(), threshold, top_n);
}

std::map<std::string, AnomalySummary> detect_mad_per_sensor(const DataSet& ds, double threshold, std::size_t top_n) {
    return run_per_sensor_detector(ds, threshold, top_n, detect_mad_for_samples);
}

} // namespace

AnomalySummary detect_anomalies_global(const DataSet& ds, AnomalyMethod method, double threshold, std::size_t top_n) {
    using enum AnomalyMethod;

    switch (method) {
    case ZScore:    return detect_zscore_global(ds, threshold, top_n);
    case IQR:       return detect_iqr_global(ds, threshold, top_n);
    case MAD:       return detect_mad_global(ds, threshold, top_n);
    }

    return {};
}

std::map<std::string, AnomalySummary> detect_anomalies_per_sensor(const DataSet& ds, AnomalyMethod method, double threshold, std::size_t top_n) {
    using enum AnomalyMethod;

    switch (method) {
    case ZScore:    return detect_zscore_per_sensor(ds, threshold, top_n);
    case IQR:       return detect_iqr_per_sensor(ds, threshold, top_n);
    case MAD:       return detect_mad_per_sensor(ds, threshold, top_n);
    }

    return {};
}

std::vector<Anomaly> select_top_anomalies(std::span<const Anomaly> anomalies, std::size_t max_count)
{
    if (max_count == 0 || anomalies.empty()) { return {}; }

    std::vector<Anomaly> result(anomalies.begin(), anomalies.end());
    const auto k = static_cast<std::ptrdiff_t>(std::min(max_count, result.size()));

    std::ranges::partial_sort(result, result.begin() + k, std::ranges::greater{},
                              [](const Anomaly& a) { return std::abs(a.score); });

    result.resize(k);
    return result;
}

} // namespace pdt
