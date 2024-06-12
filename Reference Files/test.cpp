/**
Filename    : test.cpp
Author      : Peter Freedman
Course      : CSCI 476
Date        : 11/24/23
Assignment  : Final Project  
Description : The test environment that was used for rapid iteration/debugging 
    individual sorts, as well as checking to make sure that they sorted correctly. 

    NOTE: this was partially taken from a file that no longer exists, and as such some
    documentation/content may be out of date. This file was used purely for testing, 
    and so is not necessarily up to date.
*/


//Preprocessor Macros
#define NO_FILE
//#define NO_CONTROL
#define NO_DEBUG
//#define NO_TIMING
#define NO_CHECK

//System includes
#include <concepts>     //for descriptive typenames 

//data generation
#include <random> 
#include <ranges> 
#include <vector> 
#include <algorithm>
#include <omp.h>
//output formatting/printing
#include <iostream>   
#include <string>      

#include <format>

template <typename Iter>
concept random_access = std::random_access_iterator<Iter>;
/**********************************************************************/
#include "../included/Timer.hpp"
#include "../included/BS_thread_pool.hpp"

using std::pair;
using std::vector;

template<random_access Iter, typename Value>
std::pair<Iter, Iter>
partition (Iter begin, Iter end, Value pivot);

template <random_access Iter>
void
insertionSort (Iter first, Iter last);

template <random_access Iter>
void
quickSort (Iter begin, Iter end, uint cutoff);

template<random_access Iter>
void
omp_quickSort (Iter begin, Iter end, uint cutoff, const uint minSize = 90'000);

template<typename Function>
double 
timeAlgorithm (const Function &f);

/*********************************************************************/

/** Serially sorts the range [first, last) using insertion sort
 
    @param first - the beginning of the range to be sorted
    @param end - the end (exclusive) of the range to be sorted
*/
int main()
{
    std::vector<unsigned> test(10'000'000);
    std::random_device rd;
    std::mt19937 gen{rd()};
    std::ranges::generate(test, [&] { return gen() % 100000;});
    std::vector serTest(test);
        double time = timeAlgorithm([&] {
            #pragma omp parallel 
            { 
                #pragma omp single 
                {
                    omp_quickSort(test.begin(), test.end(),15);
                }
            }
            #pragma omp taskwait
        });
  
    std::cout << time << '\n';
    double serTime = timeAlgorithm([&] {
        quickSort(serTest.begin(), serTest.end(), 15);
    });
    std::cout << serTime <<'\n';
    std::cout << std::format ("Sorts same? {}\n", test == serTest);

}

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

template<random_access Iter>
void
omp_quickSort (Iter begin, Iter end, uint cutoff, const uint minSize)
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

template<typename Function>
double 
timeAlgorithm (const Function &f)
{
    Timer t;
    f();
    t.stop();
    return t.getElapsedMs();
}