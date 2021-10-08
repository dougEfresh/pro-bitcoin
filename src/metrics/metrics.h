#ifndef METRICS_CLIENT_H
#define METRICS_CLIENT_H
#include <chain.h>
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/gauge.h>
#include <prometheus/histogram.h>
#include <prometheus/registry.h>
#include <prometheus/summary.h>
#include <validationinterface.h>
#include <net_permissions.h>
#include <util/system.h>
#include <txmempool.h>

namespace metrics {
static auto prom_registry = std::make_shared<prometheus::Registry>(); // NOLINT(cert-err58-cpp)
static std::shared_ptr<prometheus::Exposer> exposer;

enum NetDirection {
    RX,
    TX
};

enum NetConnectionType {
    TOTAL,
    SPV,
    FULL,
    INBOUND,
    OUTBOUND,
    IPV4,
    IPV6,
    TOR,
    I2P,
};

enum MemPoolType {
    POOL_SIZE,
    POOL_TRANSACTION_BYTES,
    POOl_USAGE,
    POOL_FEE
};

class Metrics
{
public:
    prometheus::Family<prometheus::Summary>& FamilySummary(const std::string&);
    prometheus::Family<prometheus::Counter>& FamilyCounter(const std::string&, const std::map<std::string, std::string>& labels = {});
    prometheus::Family<prometheus::Gauge>& FamilyGauge(const std::string&, const std::map<std::string, std::string>& labels = {});
    prometheus::Family<prometheus::Histogram>& FamilyHistory(const std::string& name, const std::map<std::string, std::string>& labels = {});
    static inline std::pair<std::string, std::string> GAUGE_LABEL = {"mtype", "gauge"};
    static inline std::pair<std::string, std::string> COUNTER_LABEL = {"mtype", "counter"};
    static inline std::pair<std::string, std::string> SUMMARY_LABEL = {"mtype", "summary"};
    static inline std::pair<std::string, std::string> HISTOGRAM_LABEL = {"mtype", "histogram"};
    std::pair<std::string, std::string> _chain_lbl;
    prometheus::Registry& _registry;
    Metrics(const std::string& chain, prometheus::Registry& registry);
};

/*
    class UtxoMetrics: Metrics {
    protected:
        prometheus::Gauge* _total_gauge;
        prometheus::Gauge* _total_out_gauge;
        prometheus::Gauge* _total_btc_amt_gauge;
        prometheus::Gauge* _db_size_gauge;
        prometheus::Gauge* _block_height_gauge;
    public:
        explicit UtxoMetrics(const std::string& chain, prometheus::Registry& registry);
        void Total(double amt);
        void Outputs(double amt);
        void BtcAmount(double amt);
        void DbSize(double amt);
        void BlockHeight(double amt);
    };
*/
struct BlockTimerOp {
    BlockTimerOp(std::string name, prometheus::Histogram::BucketBoundaries buckets);
    std::string name() const;
    prometheus::Histogram::BucketBoundaries buckets() const;
    std::string _name;
    prometheus::Histogram::BucketBoundaries _buckets;
};

static BlockTimerOp BLOCK_LOAD = BlockTimerOp("load", prometheus::Histogram::BucketBoundaries{500,5000,50000,100000});
static BlockTimerOp BLOCK_CONNECT = BlockTimerOp("connect", prometheus::Histogram::BucketBoundaries{500000, 1000000, 2000000, 4000000, 8000000});
static BlockTimerOp BLOCK_FLUSH_VIEW = BlockTimerOp("flush-view", prometheus::Histogram::BucketBoundaries{100, 200, 500, 1000});
static BlockTimerOp BLOCK_FLUSH_DISK = BlockTimerOp("flush-disk", prometheus::Histogram::BucketBoundaries{10, 25, 75, 100});
static BlockTimerOp BLOCK_UPDATE_TIP = BlockTimerOp("update-tip", prometheus::Histogram::BucketBoundaries{25, 75, 100, 200});
static BlockTimerOp BLOCK_FORK_CHK = BlockTimerOp("fork-check", prometheus::Histogram::BucketBoundaries{10000, 50000, 100000, 200000});
static BlockTimerOp BLOCK_UPDATE_INDEX = BlockTimerOp("update-index", prometheus::Histogram::BucketBoundaries{5000, 10000, 15000, 20000});

class ConfigMetrics : Metrics
{
private:
    prometheus::Family<prometheus::Gauge>* _config;
    prometheus::Gauge* _ibd;
    static std::string CategoryToString(const OptionsCategory category)
    {
        switch (category) {
        case OptionsCategory::OPTIONS:
            return "options";
            break;
        case OptionsCategory::CONNECTION:
            return "connection";
            break;
        case OptionsCategory::DEBUG_TEST:
            return "debug";
            break;
        case OptionsCategory::NODE_RELAY:
            return "node-relay";
            break;
        case OptionsCategory::BLOCK_CREATION:
            return "block";
            break;
        case OptionsCategory::RPC:
            return "rpc";
            break;
        case OptionsCategory::CHAINPARAMS:
            return "chain";
            break;
        case OptionsCategory::COMMANDS:
            return "commands";
            break;
        case OptionsCategory::REGISTER_COMMANDS:
            return "register";
            break;
        default:
            return "unknown";
            break;
        }
    }
public:
    explicit ConfigMetrics(const std::string& chain, prometheus::Registry& registry);
    void Set(const std::string& cfg, const OptionsCategory category, const std::string type, int64_t value);
    void SetFlag(const std::string& cfg, const OptionsCategory category, bool value);
    void SetIBD(const bool value);
};

class BlockMetrics
{
protected:
    std::vector<std::string> _block_types{
        "size",
        "size-witness",
        "weight",
        "height",
        "version",
        "transactions",
        "sigops",
        "time",
        "header-time",
        "fees",
        "reward",
        "difficulty"
    };
    std::map<const std::string, prometheus::Gauge*> _block_tip_gauge;
    std::vector<prometheus::Histogram*> _block_bucket_timers;
    std::vector<prometheus::Gauge*> _block_avg;
public:
    static std::unique_ptr<BlockMetrics> make(const std::string& chain, prometheus::Registry& registry, bool noop);
    //BlockMetrics() = default;
    virtual ~BlockMetrics(){};
    virtual void Size(size_t amt){};
    virtual void SizeWitness(size_t amt){};
    virtual void Height(int amt){};
    virtual void Weight(size_t amt){};
    virtual void Version(double amt){};
    virtual void Transactions(size_t amt){};
    virtual void SigOps(int64_t amt) {}
    virtual void HeaderTime(int64_t amt) {}
    virtual void Reward(int64_t amt) {};
    virtual void Fees(int64_t amt) {};
    virtual void Difficulty(double amt) {};

    virtual void TipLoadBlockDisk(int64_t current, double avg){};
    virtual void TipConnectBlock(int64_t current, double avg){};
    virtual void TipFlushView(int64_t current, double avg){};
    virtual void TipFlushDisk(int64_t current, double avg){};
    virtual void TipUpdate(int64_t current, double avg){};

    virtual void ForkCheck(int64_t current, double avg){};
    virtual void UpdateIndex(int64_t current, double avg){};
};

class BlockMetricsImpl : BlockMetrics, Metrics
{
protected:
    void set(const std::string& type, double amt);

public:
    ~BlockMetricsImpl(){};
    explicit BlockMetricsImpl(const std::string& chain, prometheus::Registry& registry);
    void Size(size_t amt) override;
    void SizeWitness(size_t amt) override;
    void Height(int amt) override;
    void Weight(size_t amt) override;
    void Version(double amt) override;
    void Transactions(size_t amt) override;
    void SigOps(int64_t amt) override;
    void HeaderTime(int64_t amt) override;
    void Reward(int64_t amt) override;
    void Fees(int64_t amt) override;
    void Difficulty(double amt) override;

    void TipLoadBlockDisk(int64_t current, double avg) override;
    void TipConnectBlock(int64_t current, double avg) override;
    void TipFlushView(int64_t current, double avg) override;
    void TipFlushDisk(int64_t current, double avg) override;
    void TipUpdate(int64_t current, double avg) override;

    void ForkCheck(int64_t current, double avg) override;
    void UpdateIndex(int64_t current, double avg) override;
};

class TxMetrics
{
protected:
    std::map<const std::string, prometheus::Counter*> _single_transaction_counter;
    prometheus::Summary* _inputs_timer;
    prometheus::Counter* _orphan_remove_counter;
    prometheus::Counter* _orphan_add_counter;
    prometheus::Counter* _accepted_counter;
    prometheus::Gauge* _cache_gauge;
    prometheus::Histogram* _check_buckets;
    prometheus::Gauge* _check_avg;

public:
    static std::unique_ptr<TxMetrics> make(const std::string& chain, prometheus::Registry& registry, bool noop);
    virtual void IncInvalid(const std::string& reason){};
    virtual void InputTime(double t){};
    virtual void IncOrphanAdd(){};
    virtual void IncOrphanRemove(){};
    virtual void IncAccepted(unsigned long amt){};
    virtual void CacheSize(double amt){};
    virtual void IncTransactions(const std::string& type, long amt){};
    virtual void TransactionCheck(int64_t current, double amt){};
};

class TxMetricsImpl : TxMetrics, Metrics
{
public:
    ~TxMetricsImpl(){};
    explicit TxMetricsImpl(const std::string& chain, prometheus::Registry& registry);
    void IncInvalid(const std::string& reason) override;
    void InputTime(double t) override;
    void IncOrphanAdd() override;
    void IncOrphanRemove() override;
    void IncAccepted(unsigned long amt) override;
    void CacheSize(double amt) override;
    void IncTransactions(const std::string& type, long amt) override;
    void TransactionCheck(int64_t current, double amt) override;
};

class NetMetrics
{
protected:
    std::map<NetConnectionType, prometheus::Gauge*> _connections_gauge;
    std::map<const std::string, prometheus::Gauge*> _bandwidth_gauge_tx;
    std::map<const std::string, prometheus::Gauge*> _bandwidth_gauge_rx;
    prometheus::Summary* _ping_timer;
    prometheus::Counter* _ping_problem_counter;
    std::map<const std::string, prometheus::Counter*> _connection_counter;
    prometheus::Gauge* _max_outbound_gauge;
    prometheus::Gauge* _max_outbound_start_gauge;

public:
    static std::unique_ptr<NetMetrics> make(const std::string& chain, prometheus::Registry& registry, bool noop);
    virtual void IncConnection(const std::string& type){};
    virtual void ConnectionGauge(NetConnectionType netConnection, uint amt){};
    virtual void BandwidthGauge(NetDirection direction, const std::string& msg, uint64_t amt){};
    virtual void PingTime(long amt){};
    virtual void IncPingProblem(){};
    virtual void MaxOutbound(int64_t amt){};
    virtual void MaxOutboundStartTime(int64_t amt){};
};
class NetMetricsImpl : NetMetrics, Metrics
{
protected:
    void initBandwidth();

public:
    ~NetMetricsImpl(){};
    explicit NetMetricsImpl(const std::string& chain, prometheus::Registry& registry);
    void IncConnection(const std::string& type) override;
    void ConnectionGauge(NetConnectionType netConnection, uint amt) override;
    void BandwidthGauge(NetDirection direction, const std::string& msg, uint64_t amt) override;
    void PingTime(long amt) override;
    void IncPingProblem() override;
    void MaxOutbound(int64_t amt) override;
    void MaxOutboundStartTime(int64_t amt) override;
};

class PeerMetrics
{
protected:
    std::vector<prometheus::Gauge*> _connections_gauge;
    std::map<const std::string, prometheus::Histogram*> _process_msg_timer;
    std::map<const std::string, prometheus::Counter*> _push_msg_counter;
    std::vector<prometheus::Counter*> _tx_validations;
    prometheus::Counter* _bad_peer_counter;
    prometheus::Counter* _misbehave_counter;
    prometheus::Gauge* _known_peers_gauge;
    prometheus::Gauge* _banned_gauge;
    prometheus::Summary* _send_msg_timer;
    std::map<NetPermissionFlags, prometheus::Gauge*> _permission_gauge;

public:
    static std::unique_ptr<PeerMetrics> make(const std::string& chain, prometheus::Registry& registry, bool noop);
    virtual void ProcessMsgType(const std::string& msg_type, long amt){};
    virtual void IncTxValidationResult(int state){};
    virtual void IncDiscourage(){};
    virtual void IncMisbehaveAmount(int amt){};
    virtual void ConnectionType(int type, uint amt){};
    virtual void Permission(NetPermissionFlags permission, uint amt){};
    virtual void Known(size_t amt){};
    virtual void SendMessageTime(long amt){};
    virtual void PushMsgType(const std::string& msg_type){};
    virtual void Banned(unsigned long amt){};
};
class PeerMetricsImpl : PeerMetrics, Metrics
{
protected:
    void initConnections();

public:
    ~PeerMetricsImpl(){};
    explicit PeerMetricsImpl(const std::string& chain, prometheus::Registry& registry);
    void ProcessMsgType(const std::string& msg_type, long amt) override;
    void IncTxValidationResult(int state) override;
    void IncDiscourage() override;
    void IncMisbehaveAmount(int amt) override;
    void ConnectionType(int type, uint amt) override;
    void Permission(NetPermissionFlags permission, uint amt) override;
    void Known(size_t amt) override;
    void SendMessageTime(long amt) override;
    void PushMsgType(const std::string& msg_type) override;
    void Banned(unsigned long amt) override;
};

class MemPoolMetrics
{
protected:
    prometheus::Histogram* _accept_pool_timer;
    std::vector<prometheus::Gauge*> _mempool_gauge;
    prometheus::Counter* _incoming_counter;
    prometheus::Counter* _vin_incoming_counter;
    prometheus::Counter* _vout_incoming_counter;
    prometheus::Counter* _incoming_size_counter;
    prometheus::Counter* _incoming_amt_counter;
    std::map<MemPoolRemovalReason, prometheus::Counter*> _removed_counter;
    prometheus::Gauge* _orphan_size_gauge;
    prometheus::Gauge* _orphan_outpoint_gauge;

public:
    static std::unique_ptr<MemPoolMetrics> make(const std::string& chain, prometheus::Registry& registry, bool noop);
    virtual void AcceptTime(long amt){};
    virtual void Transactions(MemPoolType type, long amt){};
    virtual void Incoming(size_t in, size_t out, unsigned int byte_size, int64_t amt){};
    virtual void Removed(MemPoolRemovalReason reason){};
    virtual void Orphans(size_t map, size_t outpoint){};
};
class MemPoolMetricsImpl : MemPoolMetrics, Metrics
{
public:
    explicit MemPoolMetricsImpl(const std::string& chain, prometheus::Registry& registry);
    ~MemPoolMetricsImpl(){};
    void AcceptTime(long amt) override;
    void Transactions(MemPoolType type, long amt) override;
    void Incoming(size_t in, size_t out, unsigned int byte_size, int64_t amt) override;
    void Removed(MemPoolRemovalReason reason) override;
    void Orphans(size_t map, size_t outpoint) override;
};

class MetricsNotificationsInterface final : public CValidationInterface
{
public:
    explicit MetricsNotificationsInterface(BlockMetrics& blockMetrics, MemPoolMetrics& mempoolMetrics);

protected:
    void UpdatedBlockTip(const CBlockIndex* pindexNew, const CBlockIndex* pindexFork, bool fInitialDownload) override;
    void TransactionAddedToMempool(const CTransactionRef& tx, uint64_t mempool_sequence) override;
    void TransactionRemovedFromMempool(const CTransactionRef& tx, MemPoolRemovalReason reason, uint64_t mempool_sequence) override;
    void BlockConnected(const std::shared_ptr<const CBlock>& block, const CBlockIndex* pindex) override{};
    void BlockDisconnected(const std::shared_ptr<const CBlock>& block, const CBlockIndex* pindex) override{};
    void ChainStateFlushed(const CBlockLocator& locator) override{};
    void BlockChecked(const CBlock& block, const BlockValidationState& state) override{};

private:
    BlockMetrics& _blockMetrics;
    MemPoolMetrics& _memPoolMetrics;
};
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
    std::unique_ptr<MetricsNotificationsInterface> _notifier;

public:
    Container();
    ~Container()
    {
        //            delete this;
    }
    void Init(const std::string& chain, bool noop);
    PeerMetrics& Peer();
    NetMetrics& Net();
    TxMetrics& Tx();
    BlockMetrics& Block();
    //UtxoMetrics& Utxo();
    MemPoolMetrics& MemPool();
    ConfigMetrics& Config();
    MetricsNotificationsInterface* Notifier();
};


Container* Instance();
void Init(const std::string& bind, const std::string& chain, bool noop = false);
} // namespace metrics

#endif