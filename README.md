# Servidor Web Multihilo en C

Implementación de un servidor web concurrente utilizando el modelo Productor-Consumidor y Pthreads.

## Arquitectura
- **Master Thread:** Acepta conexiones y las encola.
- **Worker Threads:** Toman conexiones de la cola y sirven archivos estáticos.
- **Sincronización:** Mutex y variables de condición para proteger el buffer circular.

## Requisitos
- GCC
- Make
- Linux/Unix (por las librerías pthread y sockets)

## Compilación y Ejecución
1. Compilar: `make`
2. Ejecutar: `./bin/servidor [puerto]`

## Pruebas
Comando para stress test:
`ab -n 1000 -c 50 http://localhost:8080/`