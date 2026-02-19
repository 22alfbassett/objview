test:
	clang++ main.cpp gl.cpp Model.cpp -o objview

prod:
	clang++ main.cpp gl.cpp Model.cpp -o objview -O3 -march=native -ffast-math -flto -DNDEBUG

debug:
	clang++ -g main.cpp gl.cpp Model.cpp -o objview

clean:
	rm ./objview