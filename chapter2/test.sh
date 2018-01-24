#!/bin/bash

echo '---------' > test.log
for f in ../testcases/*.tig
do
    ./lextest $f >> test.log
done
