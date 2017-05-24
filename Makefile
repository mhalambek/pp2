build:
	mpic++ main.cpp -std=c++14
run:
	./run.sh 1 4
test:
	g++ test.cpp -std=c++14 -o test.out
	./test.out
