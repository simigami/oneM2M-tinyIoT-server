all: server

clean:
	@rm -rf *.o
	@rm -rf server

server: main.o httpd.o onem2m.o cJSON.o
	gcc -o server $^ -ldb

main.o: main.c httpd.h onem2m.h
	gcc -c -o main.o main.c -ldb 

httpd.o: httpd.c httpd.h onem2m.h
	gcc -c -o httpd.o httpd.c -ldb

onem2m.o: onem2m.c httpd.h onem2m.h cJSON.h
	gcc -c -o onem2m.o onem2m.c -ldb

cJSON.o: cJSON.c cJSON.h
	gcc -c -o cJSON.o cJSON.c -ldb


