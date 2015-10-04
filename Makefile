#
# top level makefile for rpc library
#

base=.
include_dir=$(base)/include
test_dir=$(base)/test
example_dir=$(base)/example
profile_dir=$(base)/profile

.PHONY: all test example profile clean

all: test example profile

test:
	@make -C $(test_dir)

example:
	@make -C $(example_dir)

profile:
	@make -C $(profile_dir)

clean:
	@make clean -C $(test_dir)
	@make clean -C $(example_dir)
	@make clean -C $(profile_dir)

