# Laboratorio 2 - Sistemas Operativos
# Autores: [NOMBRE] - [RUT], [NOMBRE] - [RUT]

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
TARGET = lab2
OBJS = lab2.o funciones.o

# Regla principal: compilar el ejecutable
all: $(TARGET)

# Regla para el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Regla para lab2.o
lab2.o: lab2.c funciones.h
	$(CC) $(CFLAGS) -c lab2.c

# Regla para funciones.o
funciones.o: funciones.c funciones.h
	$(CC) $(CFLAGS) -c funciones.c

# Regla para limpiar archivos compilados
clean:
	rm -f $(OBJS) $(TARGET) *.csv *.tsv

# Regla para ejecutar prueba básica
test: $(TARGET)
	@echo "Ejecutando prueba del pipeline..."
	@echo "../L1/generator.sh -i 1 -t 5 | ../L1/preprocess.sh | ../L1/aggregate.sh | ../L1/report.sh -o test.csv" | ./$(TARGET)
	@echo "Prueba completada. Revise test.csv"

# Marcar reglas que no generan archivos
.PHONY: all clean test
