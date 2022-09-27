#!/bin/bash
socat tcp4-listen:51015,reuseaddr,fork exec:"./run-perplexity.sh"
