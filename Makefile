build:
	g++ -o auto main.cpp -IthirdParty/ -lcurl

run: build 
	./auto
