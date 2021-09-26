all: build_server build_client

build_server:
	g++ -o server server.cpp

build_client:
	g++ -o client client.cpp

clean:
	rm client server