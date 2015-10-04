//
// Some basic examples of parsing
//
// Author: Dalton Woodard
// Contact: daltonmwoodard@gmail.com
// License: Please see LICENSE.md
//

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <streambuf>
#include <utility>

#include "core/range.hpp"
#include "core/parser.hpp"
#include "core/combinators.hpp"
#include "basic/atom_parsers.hpp"
#include "basic/text_parsers.hpp"
#include "basic/regex_parsers.hpp"

using namespace rpc;

template <typename T>
using iter_type = typename std::basic_string<T>::const_iterator;

template <typename V, typename T = V>
using strparser = core::parser<iter_type<T>, V>;

strparser<std::string, char> sentence =
    rpc::core::sequence
        (rpc::basic::words<iter_type<char>>,
         rpc::core::lift
            (rpc::basic::punct<iter_type<char>>, [](char c) { return std::string (1, c); }));

auto sentences = rpc::core::some (rpc::core::ignorel (rpc::basic::spacem<iter_type<char>>, sentence));

inline bool file_exists (std::string const& filename)
{
    std::ifstream f (filename);
    return f.good();
}

std::string read_in_file (std::string const& filename)
{
    std::ifstream file (filename);
    std::string out;

    file.seekg (0, std::ios::end);
    out.reserve (file.tellg());
    file.seekg (0, std::ios::beg);

    out.assign ((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return out;
}

int main (int argc, char ** argv)
{
    std::string filename;
    std::string parse_text;

    if (argc == 1) {
        std::cout << "Need file name for text to parse!" << std::endl;
        std::exit (EXIT_FAILURE);
    }

    filename.assign (argv[1]);
    
    if (not file_exists (filename)) {
        std::cout << "File: " << filename << " does not exist (or cannot be read)!" << std::endl;
        std::exit (EXIT_FAILURE);
    }

    parse_text.assign (read_in_file (filename)); 
    std::cout << "Parsing: " << filename << " for sentences\n..." << std::endl;
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto res   = sentences.parse (parse_text);
        auto end   = std::chrono::high_resolution_clock::now();
        std::cout
            << "parse result: "
            << [](bool b) { return b ? "success" : "failure"; } (sentences.is_parse_success (res))
            << std::endl;
        std::cout << "elapsed time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
              << " microsec." << std::endl;
    }

    std::exit (EXIT_SUCCESS);
}

