#!/usr/bin/env bash
erb $(dirname $0)/test/stream_expectations.c.erb > $(dirname $0)/test/stream_expectations.c
