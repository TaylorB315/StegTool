#include "Image.h"
#include <string>
#include <iostream>

int main(int argc, char** argv) {
	Image test("../Imgs/Monkey.png");
	test.encode("this is a longer test message for fun");
	std::string message = test.decode();
	std::cout<<message<<std::endl;

	return 0;
}