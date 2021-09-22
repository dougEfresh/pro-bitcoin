#include <metrics/metrics.h>

namespace metrics {
    /*
    UtxoMetrics::UtxoMetrics(const std::string &chain, prometheus::Registry& registry): Metrics(chain, registry) {
        auto& family = FamilyGauge("utxo");
        _total_btc_amt_gauge = &family.Add({{"type", "btc-amount"}});
        _total_gauge         = &family.Add({{"type", "transactions"}});
        _total_out_gauge     = &family.Add({{"type", "outputs"}});
        _db_size_gauge       = &family.Add({{"type", "dbsize"}});
        _block_height_gauge  = &family.Add({{"type", "block-height"}});
    }

    void UtxoMetrics::Total(double amt) {
        //_total_gauge->Set(amt);
    }

    void UtxoMetrics::Outputs(double amt) {
        //_total_out_gauge->Set(amt);
    }

    void UtxoMetrics::BtcAmount(double amt) {
        //_total_btc_amt_gauge->Set(amt);
    }

    void UtxoMetrics::DbSize(double amt) {
        //_db_size_gauge->Set(amt);
    }

    void UtxoMetrics::BlockHeight(double amt) {
        //_block_height_gauge->Set(amt);
    }
*/
}