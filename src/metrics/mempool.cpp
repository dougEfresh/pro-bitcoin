#include <metrics/metrics.h>


namespace metrics {
    std::unique_ptr<MemPoolMetrics> MemPoolMetrics::make(const std::string &chain, prometheus::Registry& registry, bool noop) {
        if (noop) {
            return std::make_unique<MemPoolMetrics>();
        }
        auto m = std::make_unique<MemPoolMetricsImpl>(chain, registry);
        return reinterpret_cast<std::unique_ptr<MemPoolMetrics> &&>(m);
    }
    MemPoolMetricsImpl::MemPoolMetricsImpl(const std::string& chain, prometheus::Registry& registry): Metrics(chain, registry) {
        _accept_pool_timer = &FamilyHistory("mempool_timer",{{"method", "MempoolAcceptResult::AcceptToMemoryPoolWithTime"}})
                .Add({}, prometheus::Histogram::BucketBoundaries{ 1000, 10000, 50000, 100000} );

        auto &mempool_family = FamilyGauge("mempool", {{"method", "MemPoolAccept::AcceptSingleTransactions"}});
        auto &incoming_family = FamilyCounter("mempool_changes",
                                              {{"method", "MemPoolAccept::AcceptSingleTransactions"}});
        _mempool_gauge = {
                &mempool_family.Add({{"type", "size"}}),
                &mempool_family.Add({{"type", "bytes"}}),
                &mempool_family.Add({{"type", "usage"}}),
                &mempool_family.Add({{"type", "fee"}})
        };
        _vin_incoming_counter = &incoming_family.Add({{"type", "vin"}});
        _vout_incoming_counter = &incoming_family.Add({{"type", "vout"}});
        _incoming_size_counter = &incoming_family.Add({{"type", "bytes"}});
        _incoming_amt_counter = &incoming_family.Add({{"type", "amount"}});

        _removed_counter = {
                &incoming_family.Add({{"type",   "removed"},
                                      {"reason", "expiry"}}),
                &incoming_family.Add({{"type",   "removed"},
                                      {"reason", "size-limit"}}),
                &incoming_family.Add({{"type",   "removed"},
                                      {"reason", "reorg"}}),
                &incoming_family.Add({{"type",   "removed"},
                                      {"reason", "conflict"}}),
                &incoming_family.Add({{"type",   "removed"},
                                      {"reason", "replaced"}}),
                &incoming_family.Add({{"type",   "removed"},
                                      {"reason", "unknown"}}),
        };
        auto orphan_fmaily = &FamilyGauge("mempool_orphans", {{"method", "TxOrphanage::AddTx"}});
        _orphan_size_gauge = &orphan_fmaily->Add({{"type", "size"}});
        _orphan_outpoint_gauge = &orphan_fmaily->Add({{"type", "outpoint"}});
    }

    void MemPoolMetricsImpl::AcceptTime(long amt) {
         _accept_pool_timer->Observe((double)amt);
    }
    void MemPoolMetricsImpl::Transactions(MemPoolType type, long amt){
        this->_mempool_gauge.at(type)->Set((double)amt);
    }
    void MemPoolMetricsImpl::Incoming(size_t in, size_t out, unsigned int byte_size, int64_t amt) {
        _vin_incoming_counter->Increment((double) in);
        _vout_incoming_counter->Increment((double) out);
        _incoming_size_counter->Increment((double) byte_size);
        _incoming_amt_counter->Increment((double) amt);
    }

    void MemPoolMetricsImpl::Removed(unsigned int reason) {
        if (reason > _removed_counter.size()-2) {
            _removed_counter[_removed_counter.size()-1]->Increment();
            return;
        }
        _removed_counter[reason]->Increment();
    }
    void MemPoolMetricsImpl::Orphans(size_t map, size_t outpoint) {
        _orphan_size_gauge->Set((double)map);
        _orphan_outpoint_gauge->Set((double)outpoint);
    }
}
