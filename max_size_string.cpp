#include <iostream>
#include <string>
#include <climits>
 
int main()
{
    std::string s;
	s.append("aaaaaaaaaa");
	for (size_t i = 0; i < 1000000000 ; i++) {
			//s.append("aaaaaaaaaa");
		s = s + s;
		//s.append(s);
		std::cout << i << ": " <<  s.size() << "mb: " << s.size() / 1000000 << '\n';
	}
    std::cout
        << "Maximum size of a string is " << s.max_size() << " ("
        << std::hex << std::showbase << s.max_size()
        << "), pointer size: " << std::dec
        << CHAR_BIT*sizeof(void*) << " bits\n";

}
