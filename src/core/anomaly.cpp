#include "pdt/core/anomaly.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <span>
#include <vector>

namespace pdt {

namespace {

double median_sorted(const std::vector<double>& values) {
    // Expects values sorted in ascending order.
    // For even-sized input returns the arithmetic mean of the two middle values.
    if (values.empty()) {
        return 0.0;
    }

    const std::size_t n = values.size();
    const std::size_t mid = n / 2;

    if ((n % 2) == 0) {
        return (values[mid - 1] + values[mid]) / 2.0;
    }

    return values[mid];
}

DataSet make_dataset(std::span<const Sample> samples) {
    return DataSet{std::vector<Sample>{samples.begin(), samples.end()}};
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

    // count stores the total number of detected anomalies before trimming,
    // while top contains at most top_n anomalies sorted by score magnitude.
    summary.count = anomalies.size();

    sort_and_trim(anomalies, top_n);
    summary.top = std::move(anomalies);

    return summary;
}

AnomalySummary detect_zscore_for_samples(std::span<const Sample> samples, double threshold, std::size_t top_n) {
    if (samples.size() < 2 || threshold <= 0.0 || top_n == 0) {
        return {};
    }

    const auto stats = make_dataset(samples).stats();
    if (!(stats.stddev > 0.0)) {
        return {};
    }

    std::vector<Anomaly> anomalies;
    anomalies.reserve((samples.size() / 10) + 1); // assuming number of anomalies 10 % of number of samples

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
    // IQR rule:
    // lower = Q1 - threshold * IQR
    // upper = Q3 + threshold * IQR
    // score expresses how far the value is beyond the violated bound, normalized by IQR.
    if (samples.size() < 4 || threshold <= 0.0 || top_n == 0) {
        return {};
    }

    const auto stats = make_dataset(samples).stats();

    const double q1 = stats.q1;
    const double q3 = stats.q3;
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

    const auto stats = make_dataset(samples).stats();
    const double median = stats.median;

    std::vector<double> abs_deviations;
    abs_deviations.reserve(samples.size());

    for (const auto& sample : samples) {
        abs_deviations.push_back(std::abs(sample.value - median));
    }

    std::ranges::sort(abs_deviations);
    const double mad = median_sorted(abs_deviations);

    if (!(mad > 0.0)) {
        return {};
    }

    std::vector<Anomaly> anomalies;
    anomalies.reserve((samples.size() / 10) + 1);

    for (const auto& sample : samples) {
        const double score = (sample.value - median) / mad;

        if (std::abs(score) >= threshold) {
            anomalies.push_back(Anomaly{
                .timestamp = sample.timestamp,
                .sensor = sample.sensor,
                .value = sample.value,
                .score = score
            });
        }
    }

    return make_summary(std::move(anomalies), top_n);
}

template <typename Detector>
std::map<std::string, AnomalySummary> run_per_sensor_detector(std::span<const Sample> samples, double threshold, std::size_t top_n, Detector detector) {
    // Applies the same detector independently to samples grouped by sensor.
    std::map<std::string, AnomalySummary> result;
    const auto groups = group_by_sensor(samples);

    for (const auto& [sensor, sensor_samples] : groups) {
        result.emplace(sensor, detector(sensor_samples, threshold, top_n));
    }

    return result;
}

AnomalySummary detect_zscore_global(const DataSet& ds, double threshold, std::size_t top_n) {
    return detect_zscore_for_samples(ds.samples(), threshold, top_n);
}

std::map<std::string, AnomalySummary> detect_zscore_per_sensor(const DataSet &ds, double threshold, std::size_t top_n) {
    return run_per_sensor_detector(ds.samples(), threshold, top_n, detect_zscore_for_samples);
}

AnomalySummary detect_iqr_global(const DataSet& ds, double threshold, std::size_t top_n) {
    return detect_iqr_for_samples(ds.samples(), threshold, top_n);
}

std::map<std::string, AnomalySummary> detect_iqr_per_sensor(const DataSet& ds, double threshold, std::size_t top_n) {
    return run_per_sensor_detector(ds.samples(), threshold, top_n, detect_iqr_for_samples);
}

AnomalySummary detect_mad_global(const DataSet& ds, double threshold, std::size_t top_n) {
    return detect_mad_for_samples(ds.samples(), threshold, top_n);
}

std::map<std::string, AnomalySummary> detect_mad_per_sensor(const DataSet& ds, double threshold, std::size_t top_n) {
    return run_per_sensor_detector(ds.samples(), threshold, top_n, detect_mad_for_samples);
}

} // namespace

AnomalySummary detect_anomalies_global(const DataSet& ds, AnomalyMethod method, double threshold, std::size_t top_n) {
    using enum AnomalyMethod;

    switch (method) {
    case ZScore:
        return detect_zscore_global(ds, threshold, top_n);
    case IQR:
        return detect_iqr_global(ds, threshold, top_n);
    case MAD:
        return detect_mad_global(ds, threshold, top_n);
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
        return detect_mad_per_sensor(ds, threshold, top_n);
    }

    return {};
}

} // namespace pdt
