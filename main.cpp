#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include "tokenizer.hpp"

namespace Compiler
{

} // Compiler

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
            || argv[1] == std::string("-h")
            || argv[1] == std::string("--help"))
    {
        ShowHelp();
        return EXIT_SUCCESS;
    }

    try
    {
        Tokenizer tokenizer(); //argv[1]
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
