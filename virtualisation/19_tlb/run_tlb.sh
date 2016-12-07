#! /bin/bash

set -e

gcc -Wall -O3 tlb.c -o tlb

numtrials=$1;
outfile_base=tlb;

for outfile_num in {1..5}; do
    outfile="$outfile_base"."$numtrials"_trials.out"$outfile_num";
    echo "numpages,usecs" > $outfile;
    for numpages in 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192; do
        # Note that tlb prints a random number to stderr to avoid the compiler
        # optimising away the code we want to measure.
        # Just pipe it into the void.
        usecs=$(./tlb $numpages $numtrials 2>/dev/null)
        echo $numpages,$usecs >> $outfile
    done
done

png_files=$(Rscript make_plots.R $numtrials)

echo
echo Created the png files: $png_files

open $png_files
