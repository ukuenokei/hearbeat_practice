TARGET	= server client heartbeat
SRCS 	= server.c client.c heartbeat.c
OBJS 	= $(SRCS:.c=.o)
CC  	= gcc
LIBDIR  =
LIBS    =


# $(TARGET): $(OBJS)
# 	$(CC) -o $@ $^ $(LIBDIR)$(LIBS)

all: $(TARGET)

server: server.c 
	$(CC) -g -o $@ $^ $(LIBDIR)$(LIBS)

client: client.c 
	$(CC)  -g -o $@ $^ $(LIBDIR)$(LIBS)

heartbeat: heartbeat.c 
	$(CC)  -g -o $@ $^ $(LIBDIR)$(LIBS)

clean:
	rm -f $(OBJS) $(TARGET) *~