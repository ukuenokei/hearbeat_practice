TARGET	= test
SRCS 	= test.c
OBJS 	= $(SRCS:.c=.o)
CC  	= gcc
LIBDIR  =
LIBS    =


$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBDIR)$(LIBS)

all: $(TARGET)

clean:
	-rm -f $(OBJS) $(TARGET) *.d