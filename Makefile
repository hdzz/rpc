#
# top level makefile for funk library
#

base=.
include_dir=$(base)/include
test_dir=$(base)/test
examples_dir=$(base)/examples

CXX=clang++
std=c++14
cxxflags=$(std) $(OPTFLAGS) -Werror -Wall -Wextra -Wshadow -Wstrict-aliasing -Wcast-align

.PHONY: all test examples clean clean-all

all: test

test:
	@make -C $(test_dir)

examples:
	@make -C $(examples_dir)

clean:
	@make clean -C $(test_dir)

clean-all:
	@make clean-all -C $(test_dir)
	@make clean-all -C $(examples_dir)

