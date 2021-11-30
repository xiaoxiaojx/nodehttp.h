build:
	gcc src/main.cpp -I include/ -L lib/ -l llhttp -l uv -o output/run

start:
	./output/run