all: life_game

life.o: life.mercy
	mercy -c $< -o $@
sim.o: ../../../life_game/sim/sim.cpp
	clang++ -c $< -o $@
life_game: sim.o life.o
	clang++ $^ -lsfml-graphics -lsfml-window -lsfml-system -o $@
run: life_game
	./life_game
clean:
	rm -f *.o life_game
