
GridIdleGame: *.o *.h *.cpp
	g++ -o GridIdleGame GridIdleGame.o idlestrategy.o -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
idlestrategy.o: idlestrategy.cpp idlestrategy.h
	g++ -c idlestrategy.o idlestrategy.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
gridIdleGame.o: GridIdleGame.cpp GridIdleGame.h
	g++ -c GridIdleGame.o GridIdleGame.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
