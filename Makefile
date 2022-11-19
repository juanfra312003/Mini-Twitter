all: Gestor Cliente clean

Gestor: Gestor.o solicitud.h Usuario.h
	gcc -o Gestor Gestor.c

Gestor.o: Gestor.c solicitud.h Usuario.h
	gcc -c Gestor.c

Cliente: Cliente.o solicitud.h Usuario.h
	gcc -o Cliente Cliente.c -pthread

Cliente.o: Cliente.c solicitud.h Usuario.h
	gcc -c Cliente.c

clean:
	rm *.o
	rm pipe*