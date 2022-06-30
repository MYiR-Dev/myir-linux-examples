CC 		= ${CROSS_COMPILE}gcc
TARGET	= led-test
SRC		= led-test.c
CFLAGS	= -Wall
LDFLAGS	= 

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(CFLAGS) $(LDFLAGS) $<

.PHONY: clean
clean:
	@rm -rf $(TARGET) *.o
