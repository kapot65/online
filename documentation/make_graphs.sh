#!/bin/bash
files=$(ls graphs/*.dot graphs/*.gv)
for file in $files; do
    echo $file
    dot -Tpng $file -o ./img/$(basename $file).png
done

