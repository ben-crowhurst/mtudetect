CFLAGS += -Wall -O2
RM="rm"
OBJECTS=mtudetect.o main.o
TARGET=mtudetect

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -fPIC -D_GNU_SOURCE -o $@ $^

mtudetect.o: mtudetect.h settings.h
main.o: mtudetect.h settings.h

.PHONY: clean 
clean:
	-rm $(OBJECTS) $(TARGET)

