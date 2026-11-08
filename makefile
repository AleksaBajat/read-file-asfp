SHELL := /bin/bash

help:
	@awk 'BEGIN {FS = ":.*##"; printf "Usage: make ${FORMATTING_BEGIN_BLUE}<target>${FORMATTING_END}\n${COMPOSE_COMMAND}${FORMATTING_END}\n"} /^[a-zA-Z0-9_-]+:.*?##/ { printf "  ${FORMATTING_BEGIN_BLUE}%-46s${FORMATTING_END} %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

.PHONY:
compile: ## compile the binary
	g++ -std=c++20 -O2 -g -fno-omit-frame-pointer main.cpp -o gas_stations

.PHONY:
run: ## run the binary
	taskset -c 0 ./gas_stations

.PHONY:
perf-gen: ## generate perf report
	rm -f perf.data
	taskset -c 2 perf record -g --call-graph fp ./gas_stations

.PHONY:
verify: compile ## compare C++ output with Python reference
	./gas_stations gas_stations.csv | python3 verify.py gas_stations.csv
