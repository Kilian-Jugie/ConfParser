#include <ConfParser/confparser.hpp>
#include <ConfParser/confscope.hpp>

#include <iostream>

int main() {
	confparser::ConfParser p;
	auto d = p.Parse("test.conf");
	for(const auto& it : d->GetChilds())
		std::wcout << it->GetName() << "\n";
	return 0;
}