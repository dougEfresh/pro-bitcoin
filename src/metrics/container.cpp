#include <metrics/container.h>

namespace metrics {
Container::Container() = default;

PeerMetrics& Container::Peer()
{
    assert(this->_peerMetrics);
    return *this->_peerMetrics;
}
NetMetrics& Container::Net()
{
    assert(this->_netMetrics);
    return *this->_netMetrics;
}
TxMetrics& Container::Tx()
{
    assert(this->_txMetrics);
    return *this->_txMetrics;
}
BlockMetrics& Container::Block()
{
    assert(this->_blocks_metrics);
    return *this->_blocks_metrics;
}
MemPoolMetrics& Container::MemPool()
{
    assert(this->_mempool_metrics);
    return *this->_mempool_metrics;
}
ConfigMetrics& Container::Config()
{
    assert(this->_cfg_metrics);
    return *this->_cfg_metrics;
}

void Container::Init(const std::string& chain, bool noop)
{
    // check if already init 
    if (_init.exchange(true)) {
        return;
    }

    _peerMetrics = PeerMetrics::make(chain, *prom_registry, noop);
    _netMetrics = NetMetrics::make(chain, *prom_registry, noop);
    _txMetrics = TxMetrics::make(chain, *prom_registry, noop);
    _blocks_metrics = BlockMetrics::make(chain, *prom_registry, noop);
    //_utxo_metrics =  std::make_unique<UtxoMetrics>(chain, *prom_registry);
    _mempool_metrics = MemPoolMetrics::make(chain, *prom_registry, noop);
    _cfg_metrics = std::make_unique<ConfigMetrics>(chain, *prom_registry);
}

void Init(const std::string& bind, const std::string& chain, bool noop)
{
    if (!noop) {
        exposer = std::make_shared<prometheus::Exposer>(bind);
        exposer->RegisterCollectable(prom_registry);
    }
    Instance()->Init(chain, noop);
}

Container* Instance()
{
    static Container c;
    return &c;
}

}