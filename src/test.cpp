#include <cmath>
#include <iomanip>
#include <iostream>

#include "JSO2.h"

int main() {
	JSO2::JSO2 root;

	root["alpha"]					= 2.0;
	root["beta"]					= "PI!";
	root["gamma"]["a"]		= 0.05;
	root["gamma"]["n"]		= 2;
	root["xc"][0]["x"]		= 1.2;
	root["xc"][0]["v"]		= 0;
	root["xc"][1]["x"]		= 1.3;
	root["xc"][1]["v"]		= 0;
	root["enable"]				= true;
	root["disable"]				= false;
	root["blank-element"] = nullptr;

	std::cout << std::setprecision(15) << root << "\n";

	root.load(std::cin);

	std::cout << root << "\n";

	std::cout << "あ\n";

	return 0;
}