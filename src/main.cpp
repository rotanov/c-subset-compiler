#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>

// this_thread::sleep_for example
#include <iostream>       // std::cout
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds


#include "constants.hpp"
#include "PreTokenizer.hpp"
#include "Tokenizer.hpp"
#include "DebugPreTokenStream.hpp"
#include "DebugTokenOutputStream.hpp"
#include "SimpleExpressionParser.hpp"

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

        std::async(std::launch::async, [](){
            std::cout << "countdown:\n";
            std::chrono::seconds one_second(1);
            for (int i=10; i>0; --i) {
              std::cout << i << '\n';
              std::this_thread::sleep_for (one_second);
            }
            std::cout << "Lift off!\n";
        });

        std::async(std::launch::async, [](){
            std::cout << "countdown:\n";
            std::chrono::seconds one_second(2);
            for (int i=10; i>0; --i) {
              std::cout << i << '\n';
              std::this_thread::sleep_for (one_second);
            }
            std::cout << "Lift off!\n";
        });


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
//        DebugTokenOutputStream debugTokenOutputStream;
//        Tokenizer tokenizer(debugTokenOutputStream);
//        PreTokenizer pretokenizer(input, tokenizer);

//        simlpe expression parser AST
        SimpleExpressionParser simpleExpressionParser;
        Tokenizer tokenizer(simpleExpressionParser);
        std::thread([&] () {PreTokenizer pretokenizer(input, tokenizer);}).join();

//        std::thread f(&SimpleExpressionParser::Run, &simpleExpressionParser);
//        f.join();
        std::future<ASTNode*> rootFuture = std::async(std::launch::async,
            &SimpleExpressionParser::ParseExpression, &simpleExpressionParser);
        ASTNode* root = rootFuture.get();
        std::cout << (int)root;
//        simpleExpressionParser.Run();

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
