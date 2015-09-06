#
# top level makefile for funk library
#

base=.
include_dir=$(base)/include
test_dir=$(base)/test

CXX=clang++
std=c++14
cxxflags=$(std) $(OPTFLAGS) -Werror -Wall -Wextra -Wshadow -Wstrict-aliasing -Wcast-align

.PHONY: all test clean clean-all

all: test

test:
	@make -C $(test_dir)

clean:
	@make clean -C $(test_dir)

clean-all:
	@make clean-all -C $(test_dir)

