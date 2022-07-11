all: server

clean:
	@rm -rf *.o
	@rm -rf server

server: main.o httpd.o onem2m.o cJSON.o
	gcc -o server $^ -ldb -lpthread

main.o: main.c onem2m.h
	gcc -c -o main.o main.c 

httpd.o: httpd.c onem2m.h
	gcc -c -o httpd.o httpd.c

onem2m.o: onem2m.c onem2m.h
	gcc -c -o onem2m.o onem2m.c

cJSON.o: cJSON.c cJSON.h
	gcc -c -o cJSON.o cJSON.c


