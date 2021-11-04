#include <cassert>
#include <metrics/metrics.h>
#include <prometheus/histogram.h>

#define P_BLK_LOAD 0
#define P_BLK_CONNECT 1
#define P_BLK_FLUSH_VIEW 2
#define P_BLK_FLUSH_DISK 3
#define P_BLK_UPDATE_TIP 4
#define P_BLK_FORK_CHECK 5
#define P_BLK_INDEX 6

namespace metrics {
std::unique_ptr<BlockMetrics> BlockMetrics::make(const std::string& chain, prometheus::Registry& registry, bool noop)
{
    auto m = std::make_unique<BlockMetrics>();
    if (noop)
        return m;

    auto real = new BlockMetricsImpl(chain, registry);
    m.reset(reinterpret_cast<BlockMetrics*>(real));
    return m;
}

BlockMetricsImpl::BlockMetricsImpl(const std::string& chain, prometheus::Registry& registry) : Metrics(chain, registry)
{
    auto& family = FamilyGauge("block_tip");
    auto& block_timers = FamilyHistory("block_connect");
    auto& family_block_avg = FamilyGauge("block_avg");
    for (const auto& type : _block_types) {
        _block_tip_gauge.insert({type, &family.Add({{"type", type}})});
    }
    const std::pair<std::string, std::string> tip_method = {"method", "ConnectTip"};
    const std::pair<std::string, std::string> connect_method = {"method", "ConnectBlock"};
    _block_bucket_timers = {
        &block_timers.Add({{"operation", BLOCK_LOAD.name()}, tip_method}, BLOCK_LOAD.buckets()),
        &block_timers.Add({{"operation", BLOCK_CONNECT.name()}, tip_method}, BLOCK_CONNECT.buckets()),
        &block_timers.Add({{"operation", BLOCK_FLUSH_VIEW.name()}, tip_method}, BLOCK_FLUSH_VIEW.buckets()),
        &block_timers.Add({{"operation", BLOCK_FLUSH_DISK.name()}, tip_method}, BLOCK_FLUSH_DISK.buckets()),
        &block_timers.Add({{"operation", BLOCK_UPDATE_TIP.name()}, tip_method}, BLOCK_UPDATE_TIP.buckets()),
        &block_timers.Add({{"operation", BLOCK_FORK_CHK.name()}, connect_method}, BLOCK_FORK_CHK.buckets()),
        &block_timers.Add({{"operation", BLOCK_UPDATE_INDEX.name()}, connect_method}, BLOCK_UPDATE_INDEX.buckets()),
    };
    _block_avg = {
        &family_block_avg.Add({{"operation", BLOCK_LOAD.name()}, tip_method}),
        &family_block_avg.Add({{"operation", BLOCK_CONNECT.name()}, tip_method}),
        &family_block_avg.Add({{"operation", BLOCK_FLUSH_VIEW.name()}, tip_method}),
        &family_block_avg.Add({{"operation", BLOCK_FLUSH_DISK.name()}, tip_method}),
        &family_block_avg.Add({{"operation", BLOCK_UPDATE_TIP.name()}, tip_method}),
        &family_block_avg.Add({{"operation", BLOCK_FORK_CHK.name()}, connect_method}),
        &family_block_avg.Add({{"operation", BLOCK_UPDATE_INDEX.name()}, connect_method}),
    };
}

void BlockMetricsImpl::set(const std::string& type, double amt)
{
    auto found = this->_block_tip_gauge.find(type);
    if (found == this->_block_tip_gauge.end()) {
        return;
    }
    found->second->Set(amt);
}

void BlockMetricsImpl::Size(size_t amt)
{
    this->set("size", (double)amt);
}

void BlockMetricsImpl::SizeWitness(size_t amt)
{
    this->set("size-witness", (double)amt);
}

void BlockMetricsImpl::Height(int amt)
{
    this->set("height", (double)amt);
}

void BlockMetricsImpl::Weight(size_t amt)
{
    this->set("weight", (double)amt);
}

void BlockMetricsImpl::Version(double amt)
{
    this->set("version", (double)amt);
}

void BlockMetricsImpl::Transactions(size_t amt)
{
    this->set("transactions", (double)amt);
    auto now = std::time(nullptr);
    this->set("time", (double)now);
}

void BlockMetricsImpl::SigOps(int64_t amt)
{
    this->set("sigops", (double)amt);
}

void BlockMetricsImpl::HeaderTime(int64_t amt)
{
    this->set("header-time", (double)amt);
}

void BlockMetricsImpl::Reward(int64_t amt)
{
    this->set("reward", (double)amt);
}

void BlockMetricsImpl::Fees(int64_t amt)
{
    this->set("fees", (double)amt);
}

void BlockMetricsImpl::Difficulty(double amt)
{
    this->set("difficulty", amt);
}

void BlockMetricsImpl::ValueOut(double amt)
{
    this->set("valueout", amt);
}

void BlockMetricsImpl::TipLoadBlockDisk(int64_t current, double avg)
{
    _block_avg[P_BLK_LOAD]->Set(avg);
    _block_bucket_timers[P_BLK_LOAD]->Observe((double)current);
}
void BlockMetricsImpl::TipConnectBlock(int64_t current, double avg)
{
    _block_avg[P_BLK_CONNECT]->Set(avg);
    _block_bucket_timers[P_BLK_CONNECT]->Observe((double)current);
}
void BlockMetricsImpl::TipFlushView(int64_t current, double avg)
{
    _block_avg[P_BLK_FLUSH_VIEW]->Set(avg);
    _block_bucket_timers[P_BLK_FLUSH_VIEW]->Observe((double)current);
}
void BlockMetricsImpl::TipFlushDisk(int64_t current, double avg)
{
    _block_avg[P_BLK_FLUSH_DISK]->Set(avg);
    _block_bucket_timers[P_BLK_FLUSH_DISK]->Observe((double)current);
}
void BlockMetricsImpl::TipUpdate(int64_t current, double avg)
{
    _block_avg[P_BLK_UPDATE_TIP]->Set(avg);
    _block_bucket_timers[P_BLK_UPDATE_TIP]->Observe((double)current);
}
void BlockMetricsImpl::ForkCheck(int64_t current, double avg)
{
    _block_avg[P_BLK_FORK_CHECK]->Set(avg);
    _block_bucket_timers[P_BLK_FORK_CHECK]->Observe((double)current);
}
void BlockMetricsImpl::UpdateIndex(int64_t current, double avg)
{
    _block_avg[P_BLK_INDEX]->Set(avg);
    _block_bucket_timers[P_BLK_INDEX]->Observe((double)current);
}
} // namespace metrics