#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>

#include "constants.hpp"
#include "pretokenizer.hpp"
#include "debugpretokenstream.hpp"

void ShowHelp()
{
    std::cout <<
    R"(C language subset compiler study project.
Usage: compiler FILE
or:    compiler [OPTION]

  -h, --help, no options    display this help and exit

Author: Denis Rotanov, C8303A
)";
}

int main(int argc, char** argv)
{
    using namespace std;
    using namespace Compiler;

    if (argc == 1
            || argv[1] == string("-h")
            || argv[1] == string("--help"))
    {
        ShowHelp();
        return EXIT_SUCCESS;
    }

    try
    {
        ifstream inputFile;
        inputFile.open(argv[1], ios::binary | ios::ate);
        ifstream::pos_type fileSize = inputFile.tellg();
        inputFile.seekg(0, ios::beg);
        vector<char> input(fileSize);
        inputFile.read(&input[0], fileSize);

        DebugPreTokenStream output;
        PreTokenizer pretokenizer(input, output);
    }
    catch (exception& e)
    {
        cerr << "ERROR: " << e.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cerr << "Unhandled exception.";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
