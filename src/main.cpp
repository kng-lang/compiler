
 //-d -f c:/kng/compiler/tests/main.kng
#include "compiler.h"

#include <string>
#include <fstream>
#include <sstream>
#include <cxxopts.hpp>

int main(int argc, char** argv) {

    cxxopts::Options options("kng compiler", "The kng compiler");

    options.add_options()
        ("f,file", "File to compile", cxxopts::value<std::string>())
        ("d,debug", "Enable debugging", cxxopts::value<bool>()->default_value("false"))
        ("e,error", "Error level", cxxopts::value<u8>())
        ("t,threads", "Number of threads", cxxopts::value<u8>())
        ("h,help", "Print usage")
        ;

    auto result = options.parse(argc, argv);


    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }
    bool debug = result["debug"].as<bool>();
    if (debug)
        log("kng compiler debug enabled!");

    std::string file;
    if (result.count("file")) {
        file = result["file"].as<std::string>();
        
        Compiler compiler;

        CompileOptions options;
        if (result.count("threads")) {
            options.threads = result["threads"].as<u8>();
        }
        if (debug) {
            options.debug_emission_flags |= EMIT_TOKEN_DEBUG;
            options.debug_emission_flags |= EMIT_AST_DEBUG;
        }
        
        compiler.compile(file, options);
    }



    return 0;
}