.PHONY: test

test.bin: src/* test/*
	particle compile photon . --saveTo test.bin

local-helper: test/local-helper.cpp
	clang test/local-helper.cpp -o local-helper -lczmq

local-tester: test/local-tester.cpp
	clang test/local-tester.cpp -o local-tester -lczmq

test: test.bin local-helper
	particle flash --usb test.bin
	./local-helper
