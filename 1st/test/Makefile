OBJECTS = server.o func.o

default: $(OBJECTS)
	gcc -o server $(OBJECTS)

server.o: server.h server.c
	gcc -c server.c

func.o: server.h func.c
	gcc -c func.c

clean: 
	rm -f $(OBJECTS) server server.h.gch