/*
James Clarke - 2021
*/

//-d -f c:/kng/compiler/tests/main.kng
#include "compiler.h"

#include <string>
#include <fstream>
#include <sstream>
#include <cxxopts.hpp>


#include "exporter.h"

int main(int argc, char** argv) {

    Exporter e;
    SymTable<Type> s;
    e.export_globals(s);

    cxxopts::Options options("kng compiler", "The kng compiler");

    options.add_options()
        ("f,file", "File to compile", cxxopts::value<std::string>())
        ("b,build", "Build type [object, asm, exec]", cxxopts::value<std::string>()->default_value("object"))
        ("d,debug", "Enable debugging [l (lexer), p (parser), g (generator)]", cxxopts::value<std::string>())
        ("o,optimise", "Enable optimising [UNAVAILABLE]", cxxopts::value<u8>())
        ("e,error", "Error level [UNAVAILABLE]", cxxopts::value<u8>())
        ("t,threads", "Number of threads [UNAVAILABLE]", cxxopts::value<u8>())
        ("h,help", "Print usage")
        ;

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

        if (result.count("build")) {
            auto b = result["build"].as<std::string>();
            if (b.compare("asm")==0) {
                options.build_target = CompileOptions::BuildTarget::ASSEMBLY;
            }
            else if (b.compare("object") == 0) {
                options.build_target = CompileOptions::BuildTarget::OBJECT;
            }
            else if (b.compare("exec") == 0) {
                options.build_target = CompileOptions::BuildTarget::EXEC;
            }
        }
        if (result.count("optimise"))
            options.optimise = 1;
        if (result.count("threads")) {
            options.threads = result["threads"].as<u8>();
        }

        if (result.count("debug")) {
            if(result["debug"].as<std::string>().compare("l")==0)
                options.debug_emission_flags |= EMIT_TOKEN_DEBUG;

            else if (result["debug"].as<std::string>().compare("p") == 0)
                options.debug_emission_flags |= EMIT_AST_DEBUG;

            else if (result["debug"].as<std::string>().compare("g") == 0)
                options.debug_emission_flags |= EMIT_IR_DEBUG;
        }
        
        compiler.compile(file, options);
    }


    return 0;
}