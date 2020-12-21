#include <ConfParser/confparser.hpp>
#include <ConfParser/confscope.hpp>

#include <iostream>

int main() {
	confparser::ConfParser p;
	auto d = p.Parse("test.conf");
	std::wcout << d->GetName() << "\n";
	return 0;
}