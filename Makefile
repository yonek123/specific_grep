all: out out/specific_grep.out

out:
	mkdir out

out/specific_grep.out: src/main.cpp
	gcc -o out/specific_grep.out -fopenmp src/main.cpp -lstdc++