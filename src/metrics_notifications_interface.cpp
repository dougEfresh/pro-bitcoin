#include<metrics_notifications_interface.h>

namespace metrics {
MetricsNotificationsInterface::MetricsNotificationsInterface(BlockMetrics& blockMetrics, MemPoolMetrics& mempoolMetrics) : _blockMetrics(blockMetrics), _memPoolMetrics(mempoolMetrics) {}

void MetricsNotificationsInterface::UpdatedBlockTip(const CBlockIndex* pindexNew, const CBlockIndex* pindexFork, bool fInitialDownload)
{
    if (fInitialDownload || pindexNew == pindexFork) {
        return;
    }
    _blockMetrics.Transactions(pindexNew->nTx);
    _blockMetrics.Height(pindexNew->nHeight);
    _blockMetrics.HeaderTime(pindexNew->GetBlockHeader().GetBlockTime());
    _blockMetrics.Version(pindexNew->nVersion);
    //_blockMetrics.Difficulty(GetDifficulty(pindexNew));
}

void MetricsNotificationsInterface::TransactionAddedToMempool(const CTransactionRef& tx, uint64_t mempool_sequence)
{
    if (tx->IsNull()) {
        return;
    }
    _memPoolMetrics.Incoming(tx->vin.size(), tx->vout.size(), tx->GetTotalSize(), tx->GetValueOut());
}

void MetricsNotificationsInterface::TransactionRemovedFromMempool(const CTransactionRef& tx, MemPoolRemovalReason reason,
                                                                  uint64_t mempool_sequence)
{
    _memPoolMetrics.Removed(static_cast<size_t>(reason));
}

}