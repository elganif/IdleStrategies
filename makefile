GridIdleGame: *.o *.h *.cpp
	g++ -O2 -o GridIdleGame GridIdleGame.o olcPixelGameEngine.o entity.o maping.o factions.o button.o -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
button.o: button.h button.cpp
	g++ -O2 -c button.o button.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
factions.o: factions.h factions.cpp
	g++ -O2 -c factions.o factions.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
maping.o: maping.cpp maping.h
	g++ -O2 -c maping.o maping.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
entity.o: entity.cpp entity.h
	g++ -O2 -c entity.o entity.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
olcPixelGameEngine.o: olcPixelGameEngine.h olcPixelGameEngine.cpp
	g++ -O2 -o olcPixelGameEngine.o olcPixelGameEngine.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
gridIdleGame.o: GridIdleGame.cpp GridIdleGame.h
	g++ -O2 -c GridIdleGame.o GridIdleGame.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
