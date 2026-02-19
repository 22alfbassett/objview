CXX = clang++

test:
	$(CXX) main.cpp gl.cpp Model.cpp -o objview

prod:
	$(CXX) main.cpp gl.cpp Model.cpp -o objview -O3 -march=native -ffast-math -flto -DNDEBUG

debug:
	$(CXX) -g main.cpp gl.cpp Model.cpp -o objview

clean:
	rm ./objview