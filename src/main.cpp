#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>

#include "constants.hpp"
#include "PreTokenizer.hpp"
#include "Tokenizer.hpp"
#include "DebugPreTokenStream.hpp"
#include "DebugTokenOutputStream.hpp"

void ShowHelp()
{
    std::cout <<
    R"(C language subset compiler study project.
Usage: compiler FILE
or:    compiler [OPTION]

  -h, --help, no options    display this help and exit

Author: Denis Rotanov, B8303A
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

//        pretokenizer debug output
//        DebugPreTokenStream debugPreTokenStream;
//        PreTokenizer pretokenizer(input, debugPreTokenStream);

//        tokenizer output
        DebugTokenOutputStream debugTokenOutputStream;
        Tokenizer tokenizer(debugTokenOutputStream);
        PreTokenizer pretokenizer(input, tokenizer);
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
