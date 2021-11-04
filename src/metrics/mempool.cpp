#include <metrics/metrics.h>


namespace metrics {
std::unique_ptr<MemPoolMetrics> MemPoolMetrics::make(const std::string& chain, prometheus::Registry& registry, bool noop)
{
    auto m = std::make_unique<MemPoolMetrics>();
    if (noop)
        return m;

    auto real = new MemPoolMetricsImpl(chain, registry);
    m.reset(reinterpret_cast<MemPoolMetrics*>(real));
    return m;
}
MemPoolMetricsImpl::MemPoolMetricsImpl(const std::string& chain, prometheus::Registry& registry) : Metrics(chain, registry)
{
    _accept_pool_timer = &FamilyHistory("mempool_timer", {{"method", "MempoolAcceptResult::AcceptToMemoryPoolWithTime"}})
                              .Add({}, prometheus::Histogram::BucketBoundaries{1500, 3000, 6000, 12000});

    auto& mempool_family = FamilyGauge("mempool", {{"method", "MemPoolAccept::AcceptSingleTransactions"}});
    auto& incoming_family = FamilyCounter("mempool_changes",
                                          {{"method", "MemPoolAccept::AcceptSingleTransactions"}});
    _mempool_gauge = {
        &mempool_family.Add({{"type", "size"}}),
        &mempool_family.Add({{"type", "bytes"}}),
        &mempool_family.Add({{"type", "usage"}}),
        &mempool_family.Add({{"type", "fee"}}),
        &mempool_family.Add({{"type", "updated"}})
    };
    _vin_incoming_counter = &incoming_family.Add({{"type", "vin"}});
    _vout_incoming_counter = &incoming_family.Add({{"type", "vout"}});
    _incoming_size_counter = &incoming_family.Add({{"type", "bytes"}});
    _incoming_amt_counter = &incoming_family.Add({{"type", "amount"}});

    _removed_counter = {
        &incoming_family.Add({{"type", "removed"}, {"reason", "expiry"}}),
        &incoming_family.Add({{"type", "removed"}, {"reason", "size-limit"}}),
        &incoming_family.Add({{"type", "removed"}, {"reason", "reorg"}}),
        &incoming_family.Add({{"type", "removed"}, {"reason", "conflict"}}),
        &incoming_family.Add({{"type", "removed"}, {"reason", "replaced"}}),
    };
    _removed_unknown_counter = &incoming_family.Add({{"type", "removed"}, {"reason", "unknown"}});
    auto orphan_fmaily = &FamilyGauge("mempool_orphans", {{"method", "TxOrphanage::AddTx"}});
    _orphan_size_gauge = &orphan_fmaily->Add({{"type", "size"}});
    _orphan_outpoint_gauge = &orphan_fmaily->Add({{"type", "outpoint"}});
}

void MemPoolMetricsImpl::AcceptTime(long amt)
{
    _accept_pool_timer->Observe((double)amt);
}
void MemPoolMetricsImpl::Transactions(MemPoolType type, long amt)
{
    this->_mempool_gauge.at(type)->Set((double)amt);
}
void MemPoolMetricsImpl::Incoming(size_t in, size_t out, unsigned int byte_size, int64_t amt)
{
    _vin_incoming_counter->Increment((double)in);
    _vout_incoming_counter->Increment((double)out);
    _incoming_size_counter->Increment((double)byte_size);
    _incoming_amt_counter->Increment((double)amt);
}

void MemPoolMetricsImpl::Removed(size_t reason)
{
    if (reason > _removed_counter.size()) {
        _removed_unknown_counter->Increment();
        return;
    }
    _removed_counter[reason-1]->Increment();
}

void MemPoolMetricsImpl::Orphans(size_t map, size_t outpoint)
{
    _orphan_size_gauge->Set((double)map);
    _orphan_outpoint_gauge->Set((double)outpoint);
}
} // namespace metrics
