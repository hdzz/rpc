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
#include <list>
#include <string>
#include <streambuf>
#include <utility>

#include "core/range.hpp"
#include "core/parser.hpp"
#include "core/combinators.hpp"
#include "core/token_parsers.hpp"
#include "basic/text_parsers.hpp"
#include "basic/regex_parsers.hpp"

#ifndef RPC_PRINT_RESULTS
#define RPC_PRINT_RESULTS 0
#else
#define RPC_PRINT_RESULTS 1
#endif

using namespace rpc;
using namespace rpc::core;
using namespace rpc::basic;

using iter = typename std::basic_string<char>::const_iterator;

using sentence_type = std::deque<std::string>;

template <typename T, typename C>
auto accumulate_back = [](T const& t, C & c)
{
    c.push_back (t);
    return c;
};

template <typename T, typename C>
auto accumulate_front = [](T const& t, C & c)
{
    c.push_front (t);
    return c;
};

auto wordsep  = ignorer (word<iter>, spacem<iter>);
auto punctstr = lift
    (punct<iter>, [](char c) { return std::string (1, c); });
auto sentence = lift<sentence_type>
    (reducer (sequence (some (wordsep), punctstr),
             accumulate_front<std::string, std::deque<std::string>>,
             std::deque<std::string>{}));
auto sentencesep = ignorer (sentence, spacem<iter>);
auto sentences = some (sentencesep);

bool file_exists (std::string const& filename)
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

    out.assign ((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());
    out.erase  (1 + out.find_last_not_of (" \v\n\r\t"));
    return out;
}

template <typename T>
std::string print_results (std::deque<T> const& l)
{
    std::stringstream out;
    if (not l.empty()) {
        out << "[";
        for (auto it = l.cbegin(); it != std::prev(l.cend()); ++it)
            out << fnk::utility::to_string<T> (*it) + ", ";
        out << fnk::utility::to_string<T>(l.back()) + "]";
    } else
        out << "[]";
    return out.str();
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
        std::cout << "File: "
                  << filename
                  << " does not exist (or cannot be read)!"
                  << std::endl;
        std::exit (EXIT_FAILURE);
    }

    parse_text.assign (read_in_file (filename)); 
    std::cout << "Parsing: " << filename << " for sentences\n..." << std::endl;
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto res   = rpc::core::parse (sentences, parse_text);
        auto end   = std::chrono::high_resolution_clock::now();
        std::cout
            << "parse result: "
            << [](bool b) { return b ? "success" : "failure"; }
                    (rpc::core::parse_success (res))
            << std::endl;
        if (not rpc::core::parse_success (res)) {
            for (auto e : res)
                if (parse_failure (e)) {
                    std::cout << '\t'
                              << rpc::core::toresult_failure_message (e)
                              << " at: '"
                              << rpc::core::torange (e).grab (10)
                              << "'"
                              << std::endl;
                    std::cout << "\tparse data:"
                              << print_results (core::values(res)) << std::endl;
                }
        } else if (RPC_PRINT_RESULTS) {
            std::cout << "original data: "
                      << parse_text << std::endl;
            std::cout << "parsed data: "
                      << print_results (core::values (res)) << std::endl;
        }
        std::cout << "elapsed time: "
              << std::chrono::duration_cast<std::chrono::microseconds>
                    (end - start).count()
              << " microsec." << std::endl;
    }

    return 0;
}
