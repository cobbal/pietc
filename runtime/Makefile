pietRuntime.bc: pietRuntime.c
	clang -S -emit-llvm pietRuntime.c -o pietRuntime.s
	llvm-as pietRuntime.s -o pietRuntime.bc