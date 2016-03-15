CFLAGS += -Wall -O3
RM="rm"
OBJECTS= ping.o main.o
TARGET=mtudetect

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -fPIC -D_GNU_SOURCE -o $@ $^

mtudetect.o: ping.h settings.h
main.o: ping.h settings.h

.PHONY: clean 
clean:
	-rm $(OBJECTS) $(TARGET)
