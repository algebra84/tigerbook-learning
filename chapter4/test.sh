#!/bin/bash
echo '-------' > test.log
for f in ../testcases/*.tig
do
    ./a.out $f > test.log
done
