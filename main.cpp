
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <sstream>

using namespace std;

// Helper function declarations
vector<string> split(const string& s, char delimiter);
string trim(const string& s);
vector<string> split_arguments(const string& args_part);
string generate_cpp_code(const string& pythonCode);

void printUsage(const char* programName) {
    cerr << "Usage: " << programName << " <python_file>\n";
    cerr << "Example: " << programName << " test.py\n";
}

bool readPythonFile(const string& filename, string& content) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << "\n";
        return false;
    }
    
    content.assign(istreambuf_iterator<char>(file), {});
    return true;
}

vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

vector<string> split_arguments(const string& args_part) {
    vector<string> args;
    size_t start = 0;
    bool in_quote = false;
    char quote_char = '\0';

    for (size_t end = 0; end < args_part.size(); ++end) {
        char c = args_part[end];
        if (c == '\'' || c == '"') {
            if (!in_quote) {
                in_quote = true;
                quote_char = c;
            } else if (c == quote_char) {
                in_quote = false;
            }
        } else if (c == ',' && !in_quote) {
            args.push_back(trim(args_part.substr(start, end - start)));
            start = end + 1;
        }
    }
    args.push_back(trim(args_part.substr(start)));

    return args;
}

string generate_cpp_code(const string& pythonCode) {
    vector<string> cpp_lines = {
        "#include <iostream>",
        "#include <string>",
        "",
        "int main() {"
    };

    map<string, string> symbol_table;
    vector<string> lines = split(pythonCode, '\n');
    vector<size_t> indent_stack;

    for (size_t i = 0; i < lines.size(); ++i) {
        string line = lines[i];
        size_t line_indent = line.find_first_not_of(" \t");
        if (line_indent == string::npos) line_indent = line.size();
        string trimmed = trim(line);
        if (trimmed.empty()) continue;

        // Handle indentation changes (closing blocks)
        while (!indent_stack.empty() && line_indent < indent_stack.back()) {
            indent_stack.pop_back();
            string indent = "    " + string(indent_stack.size() * 4, ' ');
            cpp_lines.push_back(indent + "}");
        }

        // Calculate current indentation for this line
        string current_indent = "    " + string(indent_stack.size() * 4, ' ');

        // Detect if statements
        regex if_re(R"(^if\s+(.+):$)");
        smatch if_match;
        if (regex_match(trimmed, if_match, if_re)) {
            string condition = if_match[1];
            // Convert Python logical operators to C++
            condition = regex_replace(condition, regex(R"(\bnot\b)"), "!");
            condition = regex_replace(condition, regex(R"(\band\b)"), "&&");
            condition = regex_replace(condition, regex(R"(\bor\b)"), "||");

            // Generate C++ if statement
            string cpp_if = "if (" + condition + ") {";
            cpp_lines.push_back(current_indent + cpp_if);
            indent_stack.push_back(line_indent);
            continue;
        }

        // Handle print statements
        regex print_re(R"(^\s*print\s*\((.*)\)\s*$)");
        smatch print_match;
        if (regex_match(trimmed, print_match, print_re)) {
            string args_part = print_match[1];
            vector<string> args = split_arguments(args_part);
            
            if (args.empty()) continue;

            string cout_line = "std::cout";
            for (size_t i = 0; i < args.size(); ++i) {
                string arg = trim(args[i]);
                string cpp_arg;

                if ((arg.front() == '\'' && arg.back() == '\'') || 
                    (arg.front() == '"' && arg.back() == '"')) {
                    cpp_arg = "\"" + arg.substr(1, arg.size() - 2) + "\"";
                } else {
                    cpp_arg = arg;
                }

                if (i == 0) {
                    cout_line += " << " + cpp_arg;
                } else {
                    cout_line += " << \" \" << " + cpp_arg;
                }
            }
            cout_line += " << std::endl;";
            cpp_lines.push_back(current_indent + cout_line);
            continue;
        }
    }

    // Close remaining open blocks
    while (!indent_stack.empty()) {
        indent_stack.pop_back();
        string indent = "    " + string(indent_stack.size() * 4, ' ');
        cpp_lines.push_back(indent + "}");
    }

    cpp_lines.push_back("    return 0;");
    cpp_lines.push_back("}");

    string result;
    for (const auto& line : cpp_lines) {
        result += line + "\n";
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }

    string pythonCode;
    if (!readPythonFile(argv[1], pythonCode)) {
        return 1;
    }

    string cppCode = generate_cpp_code(pythonCode);
    string outFilename = "output.cpp";
    
    ofstream outFile(outFilename);
    if (!outFile) {
        cerr << "Error creating output file\n";
        return 1;
    }
    outFile << cppCode;
    outFile.close();

    cout << "Generated C++ code saved to " << outFilename << endl;
    return 0;
}