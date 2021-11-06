#include <metrics/metrics.h>
#include <protocol.h>

namespace metrics {
std::unique_ptr<PeerMetrics> PeerMetrics::make(const std::string& chain, prometheus::Registry& registry, bool noop)
{
    auto m = std::make_unique<PeerMetrics>();
    if (noop)
        return m;

    auto real = new PeerMetricsImpl(chain, registry);
    m.reset(reinterpret_cast<PeerMetrics*>(real));
    return m;
}
PeerMetricsImpl::PeerMetricsImpl(const std::string& chain, prometheus::Registry& registry) : Metrics(chain, registry)
{
    auto& netMsgTypeFamily = FamilyHistory("peer_process", {{"method", "PeerManagerImpl::ProcessMessage"}});
    auto& txValidationFamily = FamilyCounter("peer_validation_result");
    auto& pushMsgTypeFamily = FamilyCounter("peer_send", {{"method", "CConnman::PushMessage"}});
    _bad_peer_counter = &FamilyCounter("peers_discourage").Add({});
    _misbehave_counter = &FamilyCounter("peers_misbehave").Add({});

    _tx_validations = {
        &txValidationFamily.Add({{"result", "unset"}}),
        &txValidationFamily.Add({{"result", "consensus"}}),
        &txValidationFamily.Add({{"result", "recent_consensus_change"}}),
        &txValidationFamily.Add({{"result", "inputs_not_standard"}}),
        &txValidationFamily.Add({{"result", "not_standard"}}),
        &txValidationFamily.Add({{"result", "missing_inputs"}}),
        &txValidationFamily.Add({{"result", "premature_spend"}}),
        &txValidationFamily.Add({{"result", "witness_stripped"}}),
        &txValidationFamily.Add({{"result", "witness_stripped"}}),
        &txValidationFamily.Add({{"result", "conflict"}})};
    auto buckets = prometheus::Histogram::BucketBoundaries{
        5000,
        10000,
        20000,
        40000,
    };
    for (const std::string& msg : getAllNetMessageTypes()) {
        _process_msg_timer.insert({msg, &netMsgTypeFamily.Add({{"type", msg}}, buckets)});
        _push_msg_counter.insert({msg, &pushMsgTypeFamily.Add({{"type", msg}})});
    }
    _process_msg_timer.insert({"unknown", &netMsgTypeFamily.Add({{"type", "unknown"}}, buckets)});
    _push_msg_counter.insert({"unknown", &pushMsgTypeFamily.Add({{"type", "unknown"}})});
    _known_peers_gauge = &FamilyGauge("peers_known").Add({});
    _banned_gauge = &FamilyGauge("peers_banned").Add({});
    _send_msg_timer = nullptr;
    //_send_msg_timer = &FamilySummary("peer_send_time").Add({}, quantiles, window);
    auto& _permission_family = FamilyGauge("peer_permission");
    _permission_gauge = {
        {NetPermissionFlags::None, &_permission_family.Add({{"type", "none"}})},
        {NetPermissionFlags::BloomFilter, &_permission_family.Add({{"type", "bloomfilter"}})},
        {NetPermissionFlags::Relay, &_permission_family.Add({{"type", "relay"}})},
        {NetPermissionFlags::ForceRelay, &_permission_family.Add({{"type", "force-relay"}})},
        {NetPermissionFlags::Download, &_permission_family.Add({{"type", "download"}})},
        {NetPermissionFlags::NoBan, &_permission_family.Add({{"type", "noban"}})},
        {NetPermissionFlags::Mempool, &_permission_family.Add({{"type", "mempool"}})},
        {NetPermissionFlags::Addr, &_permission_family.Add({{"type", "addr"}})},
        {NetPermissionFlags::Implicit, &_permission_family.Add({{"type", "implicit"}})},
        {NetPermissionFlags::All, &_permission_family.Add({{"type", "all"}})},
    };
    initConnections();
}

// see ConnectionType in net.h
void PeerMetricsImpl::initConnections()
{
    auto& family = FamilyGauge("peer_connection");
    _connections_gauge = {
        &family.Add({{"type", "inbound"}}),
        &family.Add({{"type", "outbound-full-relay"}}),
        &family.Add({{"type", "manual"}}),
        &family.Add({{"type", "feeler"}}),
        &family.Add({{"type", "block-relay"}}),
        &family.Add({{"type", "addr-fetch"}}),
        &family.Add({{"type", "unknown"}}),
    };
}

void PeerMetricsImpl::IncTxValidationResult(int state)
{
    if (static_cast<std::size_t>(state) > _tx_validations.size() - 1) {
        return;
    }
    auto found = _tx_validations.at(state);
    found->Increment();
}

void PeerMetricsImpl::IncMisbehaveAmount(int amt)
{
    _misbehave_counter->Increment((double)amt);
}

void PeerMetricsImpl::IncDiscourage()
{
    _bad_peer_counter->Increment();
}

void PeerMetricsImpl::ProcessMsgType(const std::string& msg_type, long amt)
{
    auto found = this->_process_msg_timer.find(msg_type);
    if (found == this->_process_msg_timer.end()) {
        this->_process_msg_timer.find("unknown")->second->Observe((double)amt);
        return;
    }
    found->second->Observe((double)amt);
}

void PeerMetricsImpl::ConnectionType(int type, uint amt)
{
    if (static_cast<std::size_t>(type) > _connections_gauge.size() - 2) {
        _connections_gauge.at(_connections_gauge.size() - 2)->Set(amt);
        return;
    }
    auto found = _connections_gauge.at(type);
    found->Set(amt);
}
void PeerMetricsImpl::Permission(NetPermissionFlags permission, uint amt)
{
    auto found = this->_permission_gauge.find(permission);
    if (found == this->_permission_gauge.end()) {
        return;
    }
    found->second->Set((double)amt);
}
void PeerMetricsImpl::Known(size_t amt)
{
    _known_peers_gauge->Set((double)amt);
}
void PeerMetricsImpl::SendMessageTime(long amt)
{
    //_send_msg_timer->Observe((double )amt);
}
void PeerMetricsImpl::PushMsgType(const std::string& msg_type)
{
    auto found = this->_push_msg_counter.find(msg_type);
    if (found == this->_push_msg_counter.end()) {
        this->_push_msg_counter.find("unknown")->second->Increment();
        return;
    }
    found->second->Increment();
}
void PeerMetricsImpl::Banned(unsigned long amt)
{
    _banned_gauge->Set((double)amt);
}
} // namespace metrics
