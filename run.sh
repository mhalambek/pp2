#!/usr/bin/env bash
export TMPDIR=/tmp/parpro
mpiexec \-n $1 a.out $2
