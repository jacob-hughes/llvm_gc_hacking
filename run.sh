#!/bin/bash
./parser $1 2>&1 | sed -u 1,/BEGIN_IR/d > $2

