# Pro Bitcoin

A [promentheus](https://prometheus.io/docs/introduction/overview/) enabled bitcoin-core node running version [22.0](https://github.com/bitcoin/bitcoin/tree/v22.0), inspired by [https://statoshi.info](https://statoshi.info/?orgId=1)

## Usage

By default metrics are enabled with a scrape url at `http://localhost:8335/metrics` (18335 for testnet)

## Config

Two new configuration fields are available

| config | desc |
| ----   | --------------|
| `metrics`      | boolean 0=disable 1=enable (default) |
| `metricsbind`  | bind address and port default: localhost:8335 for main net or localhost:18335 |


## Demo

Check out [https://pro-bitcoin.io](https://pro-bitcoin.io) for a running demo with live data

## Why?

Mostly curiosity. I really wanted to explor what's really happening with bitcoin nodes, especially network communication and consensus.
I learning by tinkering and with the help of [grafana](http://docs.grafana.org/guides/getting_started/) and [prometheus](https://github.com/jupp0r/prometheus-cpp), I built a visual aid to explore the code

## Dashboards

Coming soon

### Special Thanks

* [jupp0r](https://github.com/jupp0r)
* [jlopp](https://github.com/jlopp)
