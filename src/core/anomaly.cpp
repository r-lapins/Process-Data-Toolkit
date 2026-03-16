#include "pdt/core/anomaly.h"

#include <algorithm>

namespace pdt {
namespace {

double absd(double x) { return x < 0 ? -x : x; }

// detection implementation
AnomalySummary detect_impl(std::span<const Sample> samples,
                           double mean,
                           double stddev,
                           double threshold,
                           std::size_t top_n) {
    AnomalySummary out{};

    if (samples.size() < 2) return out;
    if (!(stddev > 0.0)) return out;

    std::vector<Anomaly> anomalies;
    anomalies.reserve(samples.size() / 10 + 1);     // the assumption that up to 10% of samples may be anomalous (at least 1)

    for (const auto& s : samples) {
        double z = (s.value - mean) / stddev;
        if (absd(z) >= threshold) {
            anomalies.push_back(Anomaly{
                .timestamp = s.timestamp,
                .sensor = s.sensor,
                .value = s.value,
                .zscore = z
            });
        }
    }

    out.count = anomalies.size();

    const std::size_t n = std::min(top_n, anomalies.size());

    // top N by |z|
    std::partial_sort(anomalies.begin(), anomalies.begin() + n, anomalies.end(),
              [](const Anomaly& a, const Anomaly& b) { return absd(a.zscore) > absd(b.zscore); }  // new rule for std::sort, descending
              );

    anomalies.resize(n);
    out.top = std::move(anomalies);
    return out;
}

} // namespace

AnomalySummary detect_zscore_global(const DataSet& ds, double threshold, std::size_t top_n) {
    auto st = ds.stats();
    return detect_impl(ds.samples(), st.mean, st.stddev, threshold, top_n);
}

std::map<std::string, AnomalySummary> detect_zscore_per_sensor(const DataSet &ds, double threshold, std::size_t top_n) {
    std::map<std::string, std::vector<Sample>> groups;
    for (const auto& s : ds.samples()) {
        groups[s.sensor].push_back(s);
    }

    std::map<std::string, AnomalySummary> out;
    for (auto& [sensor, vec] : groups) {
        DataSet sub{std::move(vec)};
        auto st = sub.stats();
        out.emplace(sensor, detect_impl(sub.samples(), st.mean, st.stddev, threshold, top_n));
    }
    return out;
}

} // namespace pdt
