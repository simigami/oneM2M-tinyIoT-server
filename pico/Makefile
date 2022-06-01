all: server

clean:
	@rm -rf *.o
	@rm -rf server

server: main.o httpd.o onem2m.o
	gcc -o server $^

main.o: main.c httpd.h onem2m.h
	gcc -c -o main.o main.c

httpd.o: httpd.c httpd.h onem2m.h
	gcc -c -o httpd.o httpd.c

onem2m.o: onem2m.c httpd.h onem2m.h
	gcc -c -o onem2m.o onem2m.c

