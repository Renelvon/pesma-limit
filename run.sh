#! /bin/bash

program=pesma-limit4
make $program

exec_some_uni()
{
    echo "Running $program for $agents agents and K = $K"
    echo "=============================================="
    filename="uni-$agents-$K.txt"
    echo > $filename
    for seed in $(cat randseeds500.txt); do
         echo "Seed: $seed" | tee -a $filename
        ./$program --N $agents --n $agents --K $K --seed $seed 2>> $filename
        echo >> $filename
    done
}

exec_some_cont()
{
    echo "Running $program for $agents agents and K = $K"
    echo "=============================================="
    filename="cont10-$agents-$K.txt"
    echo > $filename
    for seed in $(cat randseeds500.txt); do
         echo "Seed: $seed" | tee -a $filename
        ./$program --N $agents --n $agents --K $K --seed $seed --p 0.1 2>> $filename
        echo >> $filename
    done
}

exec_some_inv()
{
    echo "Running $program for $agents agents and K = $K"
    echo "=============================================="
    filename="inv1-$agents-$K.txt"
    echo > $filename
    for seed in $(cat randseeds500.txt); do
         echo "Seed: $seed" | tee -a $filename
        ./$program --N $agents --n $agents --K $K --seed $seed --a 1 2>> $filename
        echo >> $filename
    done
}

for agents in 1000 2000 5000 10000 20000; do
    for p in 1 2 5 10; do
        K="$(($agents / $p))"
        exec_some_inv
        exec_some_cont
        exec_some_uni
    done
done
