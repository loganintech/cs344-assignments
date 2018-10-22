build: adventure room

test:
	./rooms.out
	./adventure.out

adventure:
	gcc --std=c89 -ggdb -pthread -o adventure.out src/sasol.adventure.c

room:
	gcc --std=c89 -ggdb -o rooms.out src/sasol.buildrooms.c

clean:
	rm adventure.out
	rm rooms.out
	rm -rf sasol.rooms.*
