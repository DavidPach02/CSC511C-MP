CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra
TARGET = simpleos
SRC = $(wildcard *.cpp)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean
