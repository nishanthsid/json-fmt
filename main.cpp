#include "include/jsonfmt.hpp"
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

bool startsWith(const string& str, const string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

unordered_map<string, string> parseArgs(int argc, char** argv) {
    unordered_map<string, string> args;
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (startsWith(arg, "--")) {
            arg = arg.substr(2);
            auto pos = arg.find('=');
            if (pos != string::npos)
                args[arg.substr(0, pos)] = arg.substr(pos + 1);
            else
                args[arg] = "true";
        } else {
            string key = (args.find("input") == args.end()) ? "input" : "output";
            args[key] = arg;
        }
    }
    return args;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "Usage:\n";
        cout << "  " << argv[0] << " <input> <output> [--minify] [--indent=<n>]\n";
        cout << "  or\n";
        cout << "  " << argv[0] << " --input=<file> --output=<file> [--minify] [--indent=<n>]\n";
        return 1;
    }

    auto args = parseArgs(argc, argv);
    if (args.find("input") == args.end() || args.find("output") == args.end()) {
        cerr << "Missing input or output file.\n";
        return 1;
    }

    string inputFile = args["input"];
    string outputFile = args["output"];
    bool minify = args.find("minify") != args.end() && args["minify"] == "true";
    int indent = 4;

    if (args.find("indent") != args.end()) {
        try {
            indent = stoi(args["indent"]);
        } catch (...) {
            cerr << "Invalid indent value, using default (4).\n";
        }
    }

    try {
        jsonfmt::JsonFormat formatter(inputFile, outputFile);
        if(minify){
            formatter.minifyJson();
        }
        else{
            formatter.formatJson(indent);
        }
    } catch (const exception& e) {
        cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
