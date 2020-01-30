all: client serve
client:
	gcc -o DUMBclient DUMBclient.c
serve:
	gcc -o DUMBserve DUMBserver.c -lpthread
clean:
	rm DUMBserve; rm DUMBclient;