#include <metrics/metrics.h>
#include <protocol.h>

namespace metrics {
    std::unique_ptr<NetMetrics> NetMetrics::make(const std::string &chain, prometheus::Registry& registry, bool noop) {
        if (noop) {
            return std::make_unique<NetMetrics>();
        }
        auto m = std::make_unique<NetMetricsImpl>(chain, registry);
        return reinterpret_cast<std::unique_ptr<NetMetrics> &&>(m);
    }
    NetMetricsImpl::NetMetricsImpl(const std::string& chain, prometheus::Registry& registry): Metrics(chain, registry) {
        initBandwidth();
        auto quantiles = prometheus::Summary::Quantiles{
            {0.50, 0.1},
            {0.90, 0.1},
            {0.99, 0.1}
        };
        auto window = std::chrono::seconds{60};
        _ping_timer = &FamilySummary("net_ping").Add({},quantiles, window);
        _ping_problem_counter = &FamilyCounter("net_ping_problem").Add({});
        auto& family_gauge = FamilyGauge("net_connection");
        _connections_gauge = {
                {NetConnectionType::TOTAL, &family_gauge.Add({{"type", "total"}})},
                {NetConnectionType::OUTBOUND, &family_gauge.Add({{"type", "outbound"}})},
                {NetConnectionType::INBOUND, &family_gauge.Add({{"type", "inbound"}})},
                {NetConnectionType::IPV4, &family_gauge.Add({{"type", "ipv4"}})},
                {NetConnectionType::IPV6, &family_gauge.Add({{"type", "ipv6"}})},
                {NetConnectionType::TOR, &family_gauge.Add({{"type", "tor"}})},
                {NetConnectionType::I2P, &family_gauge.Add({{"type", "i2p"}})},
                {NetConnectionType::FULL, &family_gauge.Add({{"type", "full"}})},
                {NetConnectionType::SPV, &family_gauge.Add({{"type", "spv"}})},
        };
        auto& family_connection_counter = FamilyCounter("net_socket");
        _connection_counter = {
                {"open",&family_connection_counter.Add({{"type", "open"}}) },
                {"accept",&family_connection_counter.Add({{"type", "accept"}}) },
                {"close",&family_connection_counter.Add({{"type", "close"}}) },
        };
    }

    void NetMetricsImpl::initBandwidth() {
        auto& family = FamilyGauge("net_bandwidth");
        _bandwidth_gauge_tx = {};
        _bandwidth_gauge_rx = {};
        for (const std::string &msg : getAllNetMessageTypes()) {
            _bandwidth_gauge_rx.insert({msg, &family.Add({{"type", msg}, {"direction", "rx"}})});
            _bandwidth_gauge_tx.insert({msg, &family.Add({{"type", msg}, {"direction", "tx"}})});
        }
        _bandwidth_gauge_rx.insert({"total", &family.Add({{"type", "total"}, {"direction", "rx"}})});
        _bandwidth_gauge_tx.insert({"total", &family.Add({{"type", "total"}, {"direction", "tx"}})});
        _bandwidth_gauge_rx.insert({"unknown", &family.Add({{"type", "unknown"}, {"direction", "rx"}})});
        _bandwidth_gauge_tx.insert({"unknown", &family.Add({{"type", "unknown"}, {"direction", "tx"}})});
    }

    void NetMetricsImpl::PingTime(long amt) {
        _ping_timer->Observe((double)amt);
    }

    void NetMetricsImpl::IncPingProblem(){
        _ping_problem_counter->Increment();
    }

    void NetMetricsImpl::BandwidthGauge(NetDirection direction, const std::string& msg, uint64_t amt) {
        if (direction == NetDirection::RX) {
            auto found = _bandwidth_gauge_rx.find(msg);
            if (found == this->_bandwidth_gauge_rx.end()) {
                this->_bandwidth_gauge_rx.find("unknown")->second->Increment((double)amt);
                return;
            }
            found->second->Increment((double)amt);
            return;
        }
        auto found = _bandwidth_gauge_tx.find(msg);
        if (found == this->_bandwidth_gauge_tx.end()) {
            this->_bandwidth_gauge_tx.find("unknown")->second->Increment((double)amt);
            return;
        }
        found->second->Increment((double)amt);
    }

    void NetMetricsImpl::ConnectionGauge(const NetConnectionType netConnection, uint amt) {
        auto found = _connections_gauge.find(netConnection);
        if (found == this->_connections_gauge.end()) {
            return;
        }
        found->second->Set(amt);
    }

    void NetMetricsImpl::IncConnection(const std::string& type) {
        auto found = _connection_counter.find(type);
        if (found == this->_connection_counter.end()) {
            return;
        }
        found->second->Increment();
    }
}
