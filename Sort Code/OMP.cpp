/*
Filename    : SoloSort.cpp
Author      : Peter Freedman
Course      : CSCI 476
Date        : 11/28/23
Assignment  : CSCI 476 - Final Project
Description : This file was modified to generate all of the executables
    in the "Executables" directory. The sorts used are listed in 
    "QuickSort.cpp". This file is currently configured for omp.
*/

/************************************************************/
// System includes
#include <iostream>
#include <concepts>

#include <random> 
#include <algorithm>

#include <omp.h>
/************************************************************/
// Local includes
#include "../CLInterpret.cpp"
#include "../included/Timer.hpp"



/************************************************************/
// Using declarations

template<typename Callable>
concept callable = std::invocable<Callable>;

template <typename Iter>
concept random_access = std::random_access_iterator<Iter>;

/************************************************************/
// Function prototypes/global vars/type definitions

template <random_access Iter>
void
omp_quickSort (Iter begin, Iter end, uint cutoff, uint minSize);

template<callable Function>
double 
timeAlgorithm (const Function &f);

std::vector<uint>
generateTestData(const unsigned size, const unsigned seed);

void
runReps (Input &in);

template <random_access Iter>
void
insertionSort (Iter first, Iter last);

template<random_access Iter, typename Value>
std::pair<Iter, Iter>
partition (Iter begin, Iter end, Value pivot);

template <random_access Iter>
void
quickSort (Iter begin, Iter end, uint cutoff);
/************************************************************/

int
main (int argc, char* argv[])
{
    Input in = compileInput(argc, argv);
    runReps(in);
}

/** This sort is the one that was replaced when exes were being generated.
    Due to the changing nature of this method, if documentation is needed 
    it can be found in the file "QuickSort.cpp".
*/
template <random_access Iter>
void
omp_quickSort (Iter begin, Iter end, uint cutoff, uint minSize)
{
    if(std::distance(begin, end) <= cutoff)
    {
        insertionSort(begin, end);
        return;
    }
    auto pivot = *begin;
    auto [lowPivot, hiPivot] = partition(begin, end, pivot);
    if(std::distance(begin, end) > minSize)
    {
        #pragma omp task default(firstprivate) shared(cutoff) 
        {
        omp_quickSort(begin, lowPivot, cutoff, minSize);
        }

        #pragma omp task default(firstprivate) shared(cutoff)
        {
        omp_quickSort(hiPivot, end, cutoff, minSize);
        }
    }
    else {
        quickSort(begin, lowPivot, cutoff);
        quickSort(hiPivot, end, cutoff);
    }
}

/** Times the algorithm passed in as a parameter

    @param f - the function to time
    @return - the time the function took to execute, as a double
*/
template<callable Function>
double 
timeAlgorithm (const Function &f)
{
    Timer t;
    f();
    t.stop();
    return t.getElapsedMs();
}

/** Generates a vector of random uints in the range [0,UNSIGNED_MAX)

    @param size - the size of the vector to be generated
    
    @return - a vector of size @p size full of elements in the range [0, 100'000'000)

    NOTE: The random numbers generated by this method will be in the same order between
    executions.
*/
std::vector<uint>
generateTestData(const unsigned size, const unsigned seed)
{
    std::vector<uint> ret(size);
    static std::mt19937 gen{seed};
    std::ranges::generate(ret, [&] { return gen();});
    return ret;
}

/** Runs trials according to user specified traits

    @param in - the user input to be used for all trials.
    
    NOTE: This method will generate the following: 
    1) in.trials * in.reps vectors of size in.vecSize
    2) # of sorts being run copies of the vectors in 1)
    3) in.trials * in.reps * # of sorts {sort, time} pairs
    over the duration of its runtime. 

    NOTE: for the sake of readability, if no CSV is generated, only 
    the data from the first 5 reps will be printed.
*/
void
runReps (Input &in)
{
    std::ofstream file(in.filename, std::ios::app);

    for(uint i = 0; i < in.reps; ++i)
    {
        std::vector<unsigned> data(in.vecSize);
        data = generateTestData (in.vecSize, in.seed);

        double time = timeAlgorithm([&] {
            #pragma omp parallel 
            {
                #pragma omp single
                {
                    omp_quickSort(data.begin(), data.end(), in.cutoff, in.vecSize * .01);
                }
            }
        });

        std::string output = std::format("{},{},{}\n", "OpenMP", time, in.vecSize);
        file.write(output.c_str(), output.length());
    }
}

/** Tests all sorts numTrials times using the following methods:
    1) takes the input vector size and adds a random value between 0 and 10000 to it
    2) takes the input cutoff and adds 1 to it on consecutive runs numTrials times
    
    @param numTrials - the number of times to repeat the above process
    @param in - the user input to use as the basis for these trials
*/
template <random_access Iter>
void
insertionSort (Iter first, Iter last)
{
    if(std::distance(first, last) < 2) { return; }

    Iter prev;
    unsigned key; 
    for(Iter cur = std::next(first); cur != last; ++cur)
    {
        //store the current value out
        key = *cur;
        prev = std::prev(cur);
        while (std::distance (first, prev) >= 0 && *prev > key)
        {
            //copy the value of prev up one
            *(std::next(prev)) = *prev;
            //decrement prev
           --prev;
        }
        *(std::next(prev)) = key;
    }
}

/** Partitions the range [begin, end) such that all elements less than *pivot 
    come before pivot, all elements equal to *pivot are in the middle, and all
    elements greater than *pivot are after towards the end.
    
    NOTE: Unstable partition
    
    @param begin - the start of the range to partition
    @param end - one past the end of the range to partition
    @param pivot - an iterator pointing to the value on which the range should 
        be partitioned
    
    @return - a pair such that all elements to the left of pair.first
            are less than pivot, all elements between pair.first and 
            pair.second are equal to pivot, and all elements after
            pair.second are greater than the pivot.
*/
template<random_access Iter, typename Value>
std::pair<Iter, Iter>
partition (Iter begin, Iter end, Value pivot)
{
    if(std::distance(begin, end) <= 1) { return {begin, end}; }
  
    Iter cur = begin;
    Iter nextLow = begin;
    Iter nextHigh = end;

    while (cur < nextHigh)
    {
        if(*cur < pivot)
        {
            std::iter_swap(cur, nextLow);
            ++nextLow;
            ++cur;
        }
        else if(*cur > pivot)
        {
            --nextHigh;
            std::iter_swap(cur, nextHigh);
        } 
        else { ++cur; }
    }

    return {nextLow, nextHigh};
}

/** Performs a dual-pivot serial quicksort on the range [begin, end),
    switching to insertion sort on smaller sample sizes.

    @param begin - the start of the range to be sorted
    @param end - the end (exclusive) of the range to be sorted
    @param cutoff - the point at which insertion sort should be used instead
    @param useOMP - if this is 1, OMP tasks will be used to parallelize the sort
        the default value of this parameter is 0.
*/
template <random_access Iter>
void
quickSort (Iter begin, Iter end, uint cutoff)
{
    if(std::distance(begin, end) <= cutoff)
    {
        insertionSort(begin, end);
        return;
    }
    auto pivot = *begin;
    auto [lowPivot, hiPivot] = partition(begin, end, pivot);

    quickSort(begin, lowPivot, cutoff);
    quickSort(hiPivot, end, cutoff);
}

