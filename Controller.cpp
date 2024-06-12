/*
Filename    : Controller.cpp
Author      : Peter Freedman
Course      : CSCI 476
Date        : 11/28/23
Assignment  : CSCI 476 - Final Project
Description : Compiles data on various quicksorts 
*/

/************************************************************/
// System includes
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

/************************************************************/
// Local includes
#include "CLInterpret.cpp"

/************************************************************/
// Using declarations



/************************************************************/
// Function prototypes/global vars/type definitions
void
runTrials (Input &in);

void
forkSort (std::string exeName, std::string inputs[], std::string clargs[]);
/************************************************************/

int
main (int argc, char* argv[])
{
    Input in = compileInput(argc, argv);
    runTrials(in);
}

/** Runs all sorts in the executables directory with the given input

    @param in - the input to be used in running the tests
*/
void
runTrials (Input &in)
{
    //the command line args
    std::string clargs[] {"vs", "ct", "nt", "rp", "st", "sd", "csv"};
    //the input data as strings
    std::string inputs[] = {std::to_string(in.vecSize).c_str(), std::to_string(in.cutoff).c_str(), std::to_string(in.trials).c_str(), std::to_string(in.reps).c_str(), std::to_string(in.stride).c_str(), std::to_string(in.seed).c_str(), in.filename.data()};

    for(uint i = 0; i < in.trials; ++i)
    {
        in.seed = i;
        std::cout << std::format ("Trial {} started.\n", i);
        inputs[5] = std::to_string(in.seed).c_str();
        inputs[0] = std::to_string(in.vecSize).c_str();

        forkSort("SerialSort", inputs, clargs);
        forkSort("JthreadSort", inputs, clargs);
        forkSort("TBBSort", inputs, clargs);
        forkSort("OMPSort", inputs, clargs);
        forkSort("BoostSort", inputs, clargs);
        forkSort("PoolSort", inputs, clargs);

        in.vecSize += in.stride;
        std::cout << std::format ("Trial {} finished\n", i);
    }
}

/** forks then calls execvp on exeName

    @param in - some of the input being passed to the process being exec'd
    @param exeName - the name of the exe to run. It is assumed to be in ./Executables
    @param inputs - the string values of the numbers to be passed as command line args.
*/
void
forkSort (std::string exeName, std::string inputs[], std::string clargs[])
{
    std::string exeRelativeFP = std::format ("./Executables/{}", exeName);
    char* args[] {exeRelativeFP.data(), clargs[0].data(), inputs[0].data(), clargs[1].data(), inputs[1].data(), clargs[2].data(), inputs[2].data(), clargs[3].data(), inputs[3].data(), clargs[4].data(), inputs[4].data(), clargs[5].data(), inputs[5].data(), clargs[6].data(), inputs[6].data(), nullptr};
    
    pid_t id = fork();
    if(id == 0)
    {
        execvp(exeRelativeFP.c_str(), args);
        exit(0);
    }
    int status;
    wait(&status);
}