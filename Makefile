all: build
build: socket_server socket_client
	
socket_server : socket_server.c
	gcc $+ -lm -o $@ 	
	
socket_client : socket_client.c
	gcc $+ -lm -o $@ 	
			
clean:  
	rm -rf socket_client socket_server



