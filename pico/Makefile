all: server

clean:
	@rm -rf *.o
	@rm -rf server

server: main.o httpd.o onem2m.o cJSON.o berkeleyDB.o jsonparse.o mqttClient.o wolfmqtt/mqtt_client.o wolfmqtt/mqtt_packet.o wolfmqtt/mqtt_socket.o logger.o util.o
	gcc -g -o server $^ -ldb -lpthread -lcurl -lm

main.o: main.c
	gcc -g -c -o main.o main.c 

httpd.o: httpd.c
	gcc -g -c -o httpd.o httpd.c

onem2m.o: onem2m.c
	gcc -g -c -o onem2m.o onem2m.c
	
berkeleyDB.o: berkeleyDB.c
	gcc -g -c -o berkeleyDB.o berkeleyDB.c
	
jsonparse.o: jsonparse.c
	gcc -g -c -o jsonparse.o jsonparse.c

cJSON.o: cJSON.c
	gcc -g -c -o cJSON.o cJSON.c

mqttClient.o: mqttClient.c
	gcc -g -c -o mqttClient.o mqttClient.c

logger.o: logger.c
	gcc -g -c -o logger.o logger.c

util.o: util.c
	gcc -g -c -o util.o util.c
