CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++20

all: kierki-serwer kierki-klient

kierki-serwer: kierki-serwer.o utilities.o
	$(CXX) $(CXXFLAGS) kierki-serwer.o utilities.o -o kierki-serwer

kierki-klient: kierki-klient.o utilities.o
	$(CXX) $(CXXFLAGS) kierki-klient.o utilities.o -o kierki-klient

kierki-serwer.o: kierki-serwer.cpp
	$(CXX) $(CXXFLAGS) -c kierki-serwer.cpp

kierki-klient.o: kierki-klient.cpp
	$(CXX) $(CXXFLAGS) -c kierki-klient.cpp

utilities.o: utilities.cpp
	$(CXX) $(CXXFLAGS) -c utilities.cpp

clean:
	rm -f kierki-serwer kierki-klient *.o *~

.PHONY: all clean
