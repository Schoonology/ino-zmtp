.PHONY: format test watch

test.bin: src/* test/*.ino
	particle compile photon . --saveTo test.bin

local-helper: test/local-helper.cpp
	clang -fno-exceptions test/local-helper.cpp -o local-helper -lczmq

local-tester: test/local-tester.cpp
	clang test/local-tester.cpp -o local-tester -lczmq

test: local-helper test.bin
	particle flash --usb test.bin
	./local-helper

format:
	clang-format -i src/* test/*

watch:
	rg --files | entr -rc make format test
