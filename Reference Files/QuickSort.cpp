/*
Filename    : QuickSort.cpp
Author      : Peter Freedman
Course      : CSCI 476
Date        : 11/13/23
Description : Contains the implementations of quicksort used 
to generate each of the executables in the executables dir. 
*/

/************************************************************/

//System includes
#include <cmath>        //std::log for depth_lvl
#include <concepts>     //for random_access 
#include <thread>       //std::jthread, std::thread::hardware_concurrency()
#include <boost/thread/scoped_thread.hpp>   //boost::scoped_thread
#include <oneapi/tbb.h> //oneapi::tbb::parallel_invoke
#include <omp.h>

/************************************************************/

// Local includes
#include "../included/BS_thread_pool.hpp"   //thread pool for parallelization


/************************************************************/

// Using declarations/concepts

//insures that an iterator is at least random access
template <typename Iter>
concept random_access = std::random_access_iterator<Iter>;


/************************************************************/

const static int depth_lvl = std::log(std::thread::hardware_concurrency());

// Function prototypes/global vars/type definitions
template <random_access Iter>
void
insertionSort (Iter first, Iter last);

template<random_access Iter, typename Value>
std::pair<Iter, Iter>
partition (Iter begin, Iter end, Value pivot);

template <random_access Iter>
void
quickSort (Iter begin, Iter end, uint cutoff);

template <typename Joining_Thread, random_access Iter>
void
quickSort (Iter begin, Iter end, uint cutoff, uint depth = depth_lvl);

template<random_access Iter>
void
quickSort (Iter begin, Iter end, uint cutoff, BS::thread_pool &threads);

template<random_access Iter>
void
tbb_quickSort (Iter begin, Iter end, uint cutoff);

template <random_access Iter>
void
omp_quickSort (Iter begin, Iter end, uint cutoff, uint minSize);

/************************************************************/

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

/** Performs a dual-pivot thread parallel quicksort on the range [begin, end)

    This quicksort will work on any type of thread that joins itself and can accept
    a callable in place of the standard (functon, args...) structure. The type of the
    thread is passed in as Joining_Thread.

    @param begin - the beginning of the range to be sorted
    @param end - the end (exclusive) of the range to be sorted
    @param cutoff - the point at which insertion sort should be used instead
    @param depth - the number of recursive calls in which new threads should be made
        NOTE: the default for this is std::log(std::thread::hardware_concurrency())
*/
template <typename Joining_Thread, random_access Iter>
void
quickSort (Iter begin, Iter end, uint cutoff, uint depth)
{
    if(std::distance(begin, end) <= cutoff)
    {
        insertionSort(begin, end);
        return;
    }

    auto pivot = *begin;
    auto [lowPivot, hiPivot] = partition(begin, end, pivot);

    if(depth > 0)
    {
        Joining_Thread t([=, &begin, &lowPivot] {quickSort<Joining_Thread>(begin, lowPivot, cutoff, depth - 1);});
        quickSort<Joining_Thread>(hiPivot, end, cutoff, depth - 1);
    }
    else {
        quickSort(begin, lowPivot, cutoff);
        quickSort(hiPivot, end, cutoff);
    }
}

/** Uses a thread pool library to quicksort the range [begin, end) in parallel

    NOTE: this method works by partitioning and then creating a new thread for
    the range [begin, lowPivot), leaving the current thread to handle the range
    [hiPivot, end). 

    @param begin - the start of the range to be sorted
    @param end - the end of the range to be sorted
    @param cutoff - the point at which insertion sort should be preferred to
                    further recursive calls
    @param threads - the thread pool to which the tasks should be submitted
*/
template<random_access Iter>
void
quickSort (Iter begin, Iter end, uint cutoff, BS::thread_pool &threads)
{
    if(std::distance(begin, end) <= cutoff)
    {
        insertionSort(begin, end);
        return;
    }

    auto pivot = *begin;
    auto [lowPivot, hiPivot] = partition(begin, end, pivot);

    threads.push_task([=, &threads] {quickSort(begin, lowPivot, cutoff, threads);});
    quickSort(hiPivot, end, cutoff);
}

/** Uses Intel's TBB library to quicksort the range [begin, end)
    More specifically, uses parallel_invoke on recursive calls to this function
    
    @param begin - the beginning of the range to sort
    @param end - the end (exclusive) of the range to sort
    @param cutoff - the point at which insertion sort should be used
*/
template<random_access Iter>
void
tbb_quickSort (Iter begin, Iter end, uint cutoff)
{
    if(std::distance(begin, end) <= cutoff)
    {
        insertionSort(begin, end);
        return;
    }
    auto pivot = *begin;
    auto [lowPivot, hiPivot] = partition(begin, end, pivot);

    oneapi::tbb::parallel_invoke(
        [=] {
            tbb_quickSort(begin, lowPivot, cutoff);
        }, 
        [=] {
            tbb_quickSort(hiPivot, end, cutoff);
        }
     );
}

/** Performs a quicksort using openMP tasks for parallelization

    @param begin - the beginning of the range to sort
    @param end - the end of the range to sort
    @param cutoff - the point to switch to insertion sort at
    @param minSize - the vec size at which task spawning should stop
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