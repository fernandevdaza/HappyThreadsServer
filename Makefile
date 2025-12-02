# Nombre del ejecutable
TARGET = bin/servidor

# Compilador y Flags
CC = gcc
# -Wall: Muestra warnings, -pthread: Para hilos, -Iinclude: Buscar headers en carpeta include
CFLAGS = -Wall -Wextra -pthread -Iinclude 

# Archivos fuente y objetos
SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, obj/%.o, $(SRCS))

# Regla principal (build)
$(TARGET): $(OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Compilación exitosa: $(TARGET)"

# Regla para compilar cada .c a .o
obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -rf bin obj
	@echo "Limpieza completada"

# Ejecutar
run: $(TARGET)
	./$(TARGET)