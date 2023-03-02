CC=gcc
RELCFLAGS = -Wall

LIBS = -ldb -lpthread -lcurl -lm

SRCS = main.c onem2m.c cJSON.c httpd.c berkeleyDB.c jsonparse.c mqttClient.c logger.c util.c wolfmqtt/mqtt_client.c wolfmqtt/mqtt_packet.c wolfmqtt/mqtt_socket.c

OBJS = $(SRCS:.c=.o)

RELEXE = server

all: $(RELEXE)

$(RELEXE) : $(OBJS)
	$(CC) $(RELCFLAGS) -c -o $(RELEXE) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(RELCFLAGS) -c $< -o $@


remake: clean all

clean:
	@rm -rf *.o
	@rm -rf server
