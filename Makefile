override CFLAGS := -Wall -Werror $(CFLAGS)
LDLIBS = -lcurl -ljson

all: json-url

clean:
	rm -f *.o json-url

.PHONY: all clean
