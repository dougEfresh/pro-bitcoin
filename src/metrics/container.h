#ifndef BITCOIN_METRICS_CONTAINER_H
#define BITCOIN_METRICS_CONTAINER_H

#include <metrics/metrics.h>

namespace metrics {
static auto prom_registry = std::make_shared<prometheus::Registry>(); // NOLINT(cert-err58-cpp)
static std::shared_ptr<prometheus::Exposer> exposer;

class Container
{
protected:
    std::unique_ptr<PeerMetrics> _peerMetrics;
    std::unique_ptr<NetMetrics> _netMetrics;
    std::unique_ptr<TxMetrics> _txMetrics;
    std::unique_ptr<BlockMetrics> _blocks_metrics;
    //std::unique_ptr<UtxoMetrics> _utxo_metrics;
    std::unique_ptr<MemPoolMetrics> _mempool_metrics;
    std::unique_ptr<ConfigMetrics> _cfg_metrics;
    std::atomic<bool> _init{false};

public:
    Container();
    ~Container()
    {
        //delete this;
    }
    void Init(const std::string& chain, bool noop);
    PeerMetrics& Peer();
    NetMetrics& Net();
    TxMetrics& Tx();
    BlockMetrics& Block();
    //UtxoMetrics& Utxo();
    MemPoolMetrics& MemPool();
    ConfigMetrics& Config();
};


void Init(const std::string& bind, const std::string& chain, bool noop = false);

Container* Instance();
}

#endif