# Makefile
# Author: Dmitry Kukovinets (d1021976@gmail.com)

# Исходники C
SRCS_C=main.c integrator.c


TARGET=integrator

#PREFIX=/urs/local/bin/


# Компиляторы
GCC=gcc -Wall

# Объектные файлы
OBJS=$(SRCS_C:.c=.o)


# Цели
.PHONY: all clear #install uninstall

all: $(TARGET)

clear:
	rm -f "$(TARGET)" *.o

#install:
#	install $(TARGET) $(PREFIX)

#uninstall:
#	rm -f $(PREFIX)/$(TARGET)

# Конечная цель
$(TARGET): $(OBJS)
	$(GCC) -o $@ $^

# Неявные преобразования
%.o: %.c
	$(GCC) -o $@ -c $<
