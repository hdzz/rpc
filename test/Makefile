#
# test level makefile for rpc library
#

base=..
include_dir=$(base)/include

test_dirs=$(wildcard ./*.test)

.PHONY: all setup clean

all: setup

setup:
	@$(foreach test, $(test_dirs), make -C $(test); )

clean:
	@rm -rf *.log *.dSYM *.DS_Store
	@$(foreach test, $(test_dirs), make -C $(test) clean; )

