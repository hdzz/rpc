#
# profile level makefile for rpc library
#

base=..
include_dir=$(base)/include
test_dir=.

source_dir=$(test_dir)/src
build_dir=$(test_dir)/build

sources=$(wildcard $(source_dir)/*.cpp)
builds=$(patsubst $(source_dir)/%, $(build_dir)/%, $(sources:.cpp=.out))

CXX=clang++
std=c++14
iflags=-I$(base) -I$(include_dir) -I$(base)/funktional/include
cxxflags=-std=$(std) $(OPTFLAGS) -O2 -Werror -Wall -Wextra -Wshadow -Wstrict-aliasing -Wcast-align -fpermissive

.PHONY: all setup clean

all: setup $(builds)

setup:
	@mkdir -p $(build_dir)

$(build_dir)/%.out : $(source_dir)/%.cpp
	$(CXX) $(iflags) $(cxxflags) $^ -o $@

clean:
	@rm -rf *.log *.dSYM *.DS_Store
	@rm -rf $(build_dir)

