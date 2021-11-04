#ifndef BITCOIN_METRICS_NOTIFICATIONS_INTERFACE_H
#define BITCOIN_METRICS_NOTIFICATIONS_INTERFACE_H

#include <validationinterface.h>
#include <metrics/metrics.h>

namespace metrics {
class MetricsNotificationsInterface final : public CValidationInterface
{
public:
    explicit MetricsNotificationsInterface(metrics::BlockMetrics& blockMetrics, metrics::MemPoolMetrics& mempoolMetrics);

protected:
    void UpdatedBlockTip(const CBlockIndex* pindexNew, const CBlockIndex* pindexFork, bool fInitialDownload) override;
    void TransactionAddedToMempool(const CTransactionRef& tx, uint64_t mempool_sequence) override;
    void TransactionRemovedFromMempool(const CTransactionRef& tx, MemPoolRemovalReason reason, uint64_t mempool_sequence) override;
    void BlockConnected(const std::shared_ptr<const CBlock>& block, const CBlockIndex* pindex) override{};
    void BlockDisconnected(const std::shared_ptr<const CBlock>& block, const CBlockIndex* pindex) override{};
    void ChainStateFlushed(const CBlockLocator& locator) override{};
    void BlockChecked(const CBlock& block, const BlockValidationState& state) override{};

private:
    metrics::BlockMetrics& _blockMetrics;
    metrics::MemPoolMetrics& _memPoolMetrics;
};
} // metrics

#endif
