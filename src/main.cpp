/*
James Clarke - 2021
*/

#include "compiler.h"

#include <string>
#include <fstream>
#include <sstream>
#include <cxxopts.hpp>


#include "exporter.h"

int main(int argc, char** argv) {

    Exporter e;
    SymTable s;
    e.export_globals(s);

    cxxopts::Options options("kng compiler", "The kng compiler");

    options.add_options()
        ("e,exec", "Build executable")
        ("f,file", "File to compile", cxxopts::value<std::string>())
        ("d,debug", "Enable debugging [l (lexer), p (parser), g (generator)]", cxxopts::value<std::string>())
        ("o,optimise", "Enable optimising [UNAVAILABLE]", cxxopts::value<u8>())
        ("w,warning", "Warning level [UNAVAILABLE]", cxxopts::value<u8>())
        ("t,threads", "Number of threads [UNAVAILABLE]", cxxopts::value<u8>())
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);


    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    std::string file;
    if (result.count("file")) {
        file = result["file"].as<std::string>();
        
        Compiler compiler;
        CompileOptions options;

        if (result.count("exec")) {
            options.m_build_target = CompileOptions::BuildTarget::OBJECT;
        }
        else {
            options.m_build_target = CompileOptions::BuildTarget::OBJECT;
        }
        if (result.count("optimise"))
            options.m_optimise = 1;
        if (result.count("threads")) {
            options.m_threads = result["threads"].as<u8>();
        }


        if (result.count("debug")) {
            if(result["debug"].as<std::string>().compare("l")==0)
                options.m_debug_flags |= EMIT_TOKEN_DEBUG;
            else if (result["debug"].as<std::string>().compare("p") == 0)
                options.m_debug_flags |= EMIT_AST_DEBUG;
            else if (result["debug"].as<std::string>().compare("g") == 0)
                options.m_debug_flags |= EMIT_IR_DEBUG;
        }
        
        compiler.compile(file, options);
    }


    return 0;
}