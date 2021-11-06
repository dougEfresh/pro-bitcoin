#include <metrics/metrics.h>

#include <future>
#include <logging.h>
#include <memory>
#include <protocol.h>
#include <utility>

namespace metrics {
using namespace prometheus;

Metrics::Metrics(const std::string& chain, Registry& registry) : _chain_lbl({"chain", chain}), _registry(registry)
{
}
Family<Gauge>& Metrics::FamilyGauge(const std::string& name, const std::map<std::string, std::string>& labels)
{
    std::map<std::string, std::string> lbls = {GAUGE_LABEL, _chain_lbl};
    for (const auto& item : labels) {
        lbls.insert(item);
    }
    return prometheus::BuildGauge()
        .Name(name)
        .Labels(lbls)
        .Register(_registry);
}
Family<Summary>& Metrics::FamilySummary(const std::string& name)
{
    return prometheus::BuildSummary()
        .Name(name)
        .Labels({SUMMARY_LABEL, _chain_lbl, {"unit", "us"}})
        .Register(_registry);
}
Family<Histogram>& Metrics::FamilyHistory(const std::string& name, const std::map<std::string, std::string>& labels)
{
    std::map<std::string, std::string> lbls = {HISTOGRAM_LABEL, _chain_lbl, {"unit", "us"}};
    for (const auto& item : labels) {
        lbls.insert(item);
    }
    return prometheus::BuildHistogram()
        .Name(name)
        .Labels(lbls)
        .Register(_registry);
}
Family<Counter>& Metrics::FamilyCounter(const std::string& name, const std::map<std::string, std::string>& labels)
{
    std::map<std::string, std::string> lbls = {COUNTER_LABEL, _chain_lbl};
    for (const auto& item : labels) {
        lbls.insert(item);
    }
    return prometheus::BuildCounter()
        .Name(name)
        .Labels(lbls)
        .Register(_registry);
}

ConfigMetrics::ConfigMetrics(const std::string& chain, prometheus::Registry& registry) : Metrics(chain, registry)
{
    auto now = std::time(nullptr);
    FamilyGauge("bitcoin_boot_time").Add({}).Set((double)now);
    _config = &FamilyGauge("bitcoin_conf");
    _ibd = &FamilyGauge("initial_block_download").Add({});
}

void ConfigMetrics::Set(const std::string& cfg, const OptionsCategory category, const std::string type, int64_t value)
{
    _config->Add({{"type", type}, {"name", cfg}, {"category", CategoryToString(category)}}).Set((double)value);
}
void ConfigMetrics::SetFlag(const std::string& cfg, const OptionsCategory category,  bool value)
{
    double flag = value ? 1.0 : 0.0;
    _config->Add({{"type", "bool"}, {"name", cfg}, {"category", CategoryToString(category)}}).Set(flag);
}
void ConfigMetrics::SetIBD(const bool value)
{
    double flag = value ? 1.0 : 0.0;
    _ibd->Set(flag);
}


BlockTimerOp::BlockTimerOp(std::string name, prometheus::Histogram::BucketBoundaries buckets) : _name(std::move(name)), _buckets(std::move(buckets)) {}

std::string BlockTimerOp::name() const
{
    return _name;
}
prometheus::Histogram::BucketBoundaries BlockTimerOp::buckets() const
{
    return _buckets;
}
} // namespace metrics