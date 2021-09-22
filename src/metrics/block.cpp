#include <metrics/metrics.h>
#include <cassert>
#include <prometheus/histogram.h>

#define P_BLK_LOAD 0
#define P_BLK_CONNECT 1
#define P_BLK_FLUSH_VIEW 2
#define P_BLK_FLUSH_DISK 3
#define P_BLK_UPDATE_TIP 4
#define P_BLK_FORK_CHECK 5
#define P_BLK_INDEX 6

namespace metrics {
    std::unique_ptr<BlockMetrics> BlockMetrics::make(const std::string &chain, prometheus::Registry& registry, bool noop) {
        if (noop) {
            return std::make_unique<BlockMetrics>();
        }
        auto m = std::make_unique<BlockMetricsImpl>(chain, registry);
        return reinterpret_cast<std::unique_ptr<BlockMetrics> &&>(m);
    }

    BlockMetricsImpl::BlockMetricsImpl(const std::string &chain, prometheus::Registry& registry): Metrics(chain, registry) {
        auto& family = FamilyGauge("block_tip");
        auto& block_timers = FamilyHistory("block_connect");
        auto& family_block_avg = FamilyGauge("block_avg");
        _block_tip_gauge = {};
        for (const auto &type : _block_types) {
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

    void BlockMetricsImpl::set(const std::string& type, double amt) {
        auto found = this->_block_tip_gauge.find(type);
        if (found == this->_block_tip_gauge.end()) {
            return;
        }
        found->second->Set(amt);
    }

    void BlockMetricsImpl::Size(double amt) {
        this->set("size", amt);
    }

    void BlockMetricsImpl::SizeWitness(double amt) {
        this->set("size-witness", amt);
    }

    void BlockMetricsImpl::Height(double amt) {
        this->set("height", amt);
    }

    void BlockMetricsImpl::Weight(double amt) {
        this->set("weight", amt);
    }

    void BlockMetricsImpl::Version(double amt) {
        this->set("version", amt);
    }

    void BlockMetricsImpl::Transactions(double amt) {
        this->set("transactions", amt);
    }

    void BlockMetricsImpl::SigOps(double amt) {
        this->set("sigops", amt);
    }

    void BlockMetricsImpl::TipLoadBlockDisk(int64_t current, double avg) {
        _block_avg[P_BLK_LOAD]->Set(avg);
        _block_bucket_timers[P_BLK_LOAD]->Observe((double)current);
    }
    void BlockMetricsImpl::TipConnectBlock(int64_t current, double avg) {
        _block_avg[P_BLK_CONNECT]->Set(avg);
        _block_bucket_timers[P_BLK_CONNECT]->Observe((double)current);
    }
    void BlockMetricsImpl::TipFlushView(int64_t current, double avg) {
        _block_avg[P_BLK_FLUSH_VIEW]->Set(avg);
        _block_bucket_timers[P_BLK_FLUSH_VIEW]->Observe((double)current);
    }
    void BlockMetricsImpl::TipFlushDisk(int64_t current, double avg) {
        _block_avg[P_BLK_FLUSH_DISK]->Set(avg);
        _block_bucket_timers[P_BLK_FLUSH_DISK]->Observe((double)current);
    }
    void BlockMetricsImpl::TipUpdate(int64_t current, double avg) {
        _block_avg[P_BLK_UPDATE_TIP]->Set(avg);
        _block_bucket_timers[P_BLK_UPDATE_TIP]->Observe((double)current);
    }
    void BlockMetricsImpl::ForkCheck(int64_t current, double avg) {
        _block_avg[P_BLK_FORK_CHECK]->Set(avg);
        _block_bucket_timers[P_BLK_FORK_CHECK]->Observe((double)current);
    }
    void BlockMetricsImpl::UpdateIndex(int64_t current, double avg) {
        _block_avg[P_BLK_INDEX]->Set(avg);
        _block_bucket_timers[P_BLK_INDEX]->Observe((double)current);
    }
}