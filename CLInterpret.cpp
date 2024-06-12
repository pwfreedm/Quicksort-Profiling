/**
Filename    : CLInterpret.cpp
Author      : Peter Freedman
Course      : CSCI 476
Date        : 11/24/23
Assignment  : Final Project  
Description : A command line interface to the class QuickSort.cpp. 
              parallelizes multiple instances of single-threaded tasks.
    NOTE: Behavior is undefined if the trial count, reps, or vector size are <= zero.
    NOTE: Behavior is undefined if the csv flag is used without a 
    filename, the filename is not valid, or the filename contains an 
    extension.
*/

/************************************************************/
// System includes

#include <iostream> //terminal I/O
#include <format>   //output formatting
#include <string>   //sent to output
#include <bitset>   //flags

//file output
#include <cstring>

/************************************************************/
// Local includes

/************************************************************/
// Using declarations

using uint = unsigned int;
/************************************************************/
// Function prototypes/global vars/type definitions

const static int flagCount = 7;
std::bitset<flagCount> flags;

//aliases for readability/maintainability
const static int VECSIZE = 0;
const static int CUTOFF = 1;
const static int NUM_TRIALS = 2;
const static int REPS_PER_TRIAL = 3;
const static int STRIDE = 4;
const static int SEED = 5;
const static int WRITE_CSV = 6;

/** Container for all the input the user is asked for. 
    NOTE: Seed is incremented automatically between trials
    NOTE: filename is not prompted for, and must be entered via cli
*/
struct Input 
{
    std::string filename{};
    uint vecSize{};
    uint cutoff{};
    uint trials{};
    uint stride{};
    uint reps{};
    uint seed{};
};

Input 
compileInput (int argc, char* argv[]);

void
printHelpMenu();

Input
parseArgs(int argc, char* argv[]);

void 
validateInput (Input &in);

int 
tryNumericArg(int arg, const std::string &test, const std::string &argName);

/************************************************************/

/** Compiles an input object by checking the cli args and prompting
    for anything missing. 
    
    @param argc - the number of arguments to parse
    @param argv - the pointer to an array of string args
    
    @return - an object of type Input containing all user input needed to 
    run trials
*/
Input
compileInput (int argc, char* argv[])
{
    if(argc > 1 && strcmp("help", argv[1]) == 0)
    {
        printHelpMenu();
        exit(EXIT_SUCCESS);
    }
    Input input = parseArgs(argc, argv);
    validateInput(input);
    return input;
}

/** Prints the help menu that appears when ran with "help"*/
void
printHelpMenu()
{
    std::cout << "Input flags: \n"
              << "     vs  #  - the vector size to generate\n"
              << "     ct  #  - the point to switch to insertion sort (cutoff)\n"
              << "     nt  #  - the number of trials to run\n"
              << "     rp  #  - the number of times to run each trial\n"
              << "     st  #  - the stride to increase the vector size by between trials\n"
              << "     sd  #  - the seed to be used in the first trial. Incremented between trials\n"
              << "Output flags: \n"
              << "     csv n  - write raw data to file n.csv instead of stdout\n";
}

/** Parses the command line args input by the user, updating a bitset 
    to reflect which flags were provided and providing some basic error 
    handling along the way. 
    
    @param argc - the argument count to the executable.
    @param argv - the argument list, with @p argc elements in it.
    
    @return an Input object containing all of the arguments passed to flags
    which require args.
*/
Input
parseArgs(int argc, char* argv[])
{
    const char* args[] {"vs", "ct", "nt", "rp", "st", "sd", "csv"};
    Input in;

    //skip first arg because it is executable name
    for(int arg = 1; arg < argc; ++arg)
    {
        if(strcmp(args[VECSIZE], argv[arg]) == 0)
        {
            in.vecSize = tryNumericArg(VECSIZE, argv[++arg], "vecsize");
        }
        else if(strcmp(args[CUTOFF], argv[arg]) == 0)
        {
            in.cutoff = tryNumericArg(CUTOFF, argv[++arg], "cutoff");
        }
        else if(strcmp(args[NUM_TRIALS], argv[arg]) == 0)
        {
            in.trials = tryNumericArg(NUM_TRIALS, argv[++arg], "trial count");
        }
        else if(strcmp(args[REPS_PER_TRIAL], argv[arg]) == 0)
        {
            in.reps = tryNumericArg(REPS_PER_TRIAL, argv[++arg], "repetitions per trial");
        }
        else if(strcmp(args[STRIDE], argv[arg]) == 0)
        {
            in.stride = tryNumericArg(STRIDE, argv[++arg], "stride");
        }
        else if(strcmp(args[SEED], argv[arg]) == 0)
        {
            in.seed = tryNumericArg(SEED, argv[++arg], "seed");
        }
        else if(strcmp(args[WRITE_CSV], argv[arg]) == 0)
        {
            in.filename = argv[++arg];
            flags[WRITE_CSV] = 1;
        }
        //if this case is reached, the flag is invalid
        else {
        {
            std::cerr << std::format("'{}' is not recognized. Try again or run with help to list flags.\n", argv[arg]);
            exit(EXIT_FAILURE);
        }
        }
    }
    return in;
}

/** Makes sure that the input given in the flags is valid, prompting for 
    whatever is missing.
    
    @param in - the input to be validated
*/
void 
validateInput (Input &in)
{
    if (in.cutoff == 0)
    {
        std::cout << "cutoff: ";
        std::cin >> in.cutoff;
    }
    if(in.trials == 0)
    {
        std::cout << "num trials: ";
        std::cin >> in.trials;
    }
    if(in.vecSize == 0)
    {
        std::cout << "num elements: ";
        std::cin >> in.vecSize;
    }
    if(in.reps == 0)
    {
        std::cout << "runs per trial: ";
        std::cin >> in.reps;
    }
    if(in.stride == 0)
    {
        std::cout << "vector growth per trial: ";
        std::cin >> in.stride;
    }
}

/** Attempts to convert a single argument to an int.

    @param arg - the code of the arg being converted
    @param test - the argument (from argv) to convert
    @param argName - the name of the argument. For printing an error if
        conversion fails
    
    @return - the value of @p test if the conversion was successful
*/
int 
tryNumericArg(int arg, const std::string &test, const std::string &argName)
{
    int val = 0;
    try {
        val = std::stoi(test);
        flags[arg] = 1;
    } 
    catch (std::exception &e)
    {
        std::cerr << std::format("The {} flag must be followed by a uint size.\n", argName);
        exit(EXIT_FAILURE);
    }
    return val;
}