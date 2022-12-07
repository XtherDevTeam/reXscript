#include <iostream>
#include "share/argparse.hpp"
#include <rex.hpp>

void interactiveShell(rex::managedPtr<rex::environment> &env) {
    auto moduleCxt = rex::managePtr(rex::value{rex::value::cxtObject{}});
    env->globalCxt->members[L"__local__"] = moduleCxt;
    auto interpreter = rex::managePtr(rex::interpreter{env, moduleCxt});

    interpreter->stack.emplace_back();
    interpreter->stack.back().pushLocalCxt({});
    while (std::cin) {
        std::string buf;
        std::cout << "input> ";
        std::getline(std::cin, buf);
        std::wstring code;
        code = std::move(rex::string2wstring(buf));
        rex::stringToLexerPass pass1(code);
        rex::lexer lexer = pass1.run();
        rex::parser parser{lexer};
        rex::AST ast = parser.parseStmts();
        rex::value result = interpreter->interpret(ast);
        std::cout << "output> " << rex::wstring2string(result) << std::endl;
    }
}

void loadFile(rex::managedPtr<rex::environment> &env, const rex::vstr &path) {
    auto moduleCxt = rex::importExternModules(env, path);
}

int main(int argc, const char **argv) {
    argparse::ArgumentParser rexProg{"rex"};
    rexProg.add_argument("--shell")
            .help("open interactive shell")
            .default_value(false)
            .implicit_value(true);

    rexProg.add_argument("--file")
            .help("specify the file to be execute")
            .default_value(std::string{});

    try {
        rexProg.parse_args(argc, argv);
        auto env = rex::getRexEnvironment();
        if (rexProg["--shell"] == true) {
            interactiveShell(env);
        } else if (!rexProg.get<std::string>("--file").empty()) {
            loadFile(env, rex::string2wstring(rexProg.get<std::string>("--file")));
        } else {
            std::cout << rexProg;
        }
    } catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cout << rexProg;
        std::exit(1);
    }
    return 0;
}
