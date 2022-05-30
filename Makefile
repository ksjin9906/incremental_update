run.out : Baum.o file.o func.o ups.o Node.o upstable.o
	g++ -o run.out -O2 Baum.o file.o func.o ups.o Node.o upstable.o

Baum.o: Baum_v8.cpp
	g++ -c -o Baum.o -O2 Baum_v8.cpp

file.o: file.cpp
	g++ -c -o file.o -O2 file.cpp

func.o: func.cpp
	g++ -c -o func.o -O2 func.cpp

ups.o: ups.cpp
	g++ -c -o ups.o -O2 ups.cpp

Node.o: Node.cpp
	g++ -c -o Node.o -O2 Node.cpp

upstble.o: upstable.cpp
	g++ -c -o upstable.o -O2 upstable.cpp

clean :
	rm *.o run.out
