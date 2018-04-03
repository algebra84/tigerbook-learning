#!/bin/bash
echo '-------' > test.log
for f in ../testcases/*.tig
do
    #    echo -e '\n' >> test.log
    #    echo '--------' $f >> test.log
    #    echo -e '\n' >> test.log
    echo -e '\n'
    echo -e $f
    echo -e '\n'
    ./tcc $f >> test.log
done
