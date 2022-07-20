#include "Utils.hpp"

std::vector<std::string> list_dir(std::string path) {
	DIR						 *dir;
	struct dirent			 *ent;
	std::vector<std::string> listing;

	if ((dir = opendir (path.c_str())) != nullptr) {
		while ((ent = readdir(dir)) != nullptr) {
			std::string entry(ent->d_name);
			if (entry == "." || entry == "..")
				continue;
			listing.push_back(entry);
		}
		closedir(dir);
		return listing;
	}
	perror("error list dir");
	return listing;

}

size_t get_file_size(std::string & path) {
	struct stat s;
	size_t sz = 0;
	if (!stat( path.c_str(), &s ) )
		sz = s.st_size;
	return sz;
}

bool valid_dir_path(std::string path) {
	struct stat s;
	if( stat( path.c_str(), &s ) == EXIT_SUCCESS && (s.st_mode & S_IFDIR ))
		return true;
	return false;
}

// recursive search
bool find_file(std::vector<std::string> & filenames,
			   std::string path, // onli dir
			   std::string & result) {
//	std::cout << "\tsearching in dir: " << path << "\n";
	std::vector<std::string> listing = list_dir(path);
	std::vector<std::string> dirs;
	for ( int i=0; i < listing.size(); i++ )
	{
		std::string new_path = path + "/" + listing[i];
		if ( easyfind(filenames, listing[i]) ) {
			result = new_path;
			return true;
		} else {
			if ( valid_dir_path(new_path) )
				dirs.push_back(new_path);
		}

	}
	for ( int i=0; i < dirs.size(); i++ ) {
		if ( find_file(filenames, dirs[i], result) )
			return true;
	}
	return false;
}

bool validate_hex( std::string & number ) {
	for ( int i = 0; i < number.size(); i++ ) {
		if ( !isdigit(number[i])
			 && ( std::toupper(number[i]) > 70 || std::toupper(number[i]) < 65 ) )
			return false;
	}
	return true;
}
#ifdef CVER
size_t hextoi(const std::string & number) { // case sansitive
	size_t res = 0;
	int j = 0;
	for (int i = number.size() - 1; i >= 0; i--, j++) {
		int n = number[i] - 48;
		//std::cout << n << ' ';
		if ( n > 10 )
			n -= 7;
		res += pow(16, j) * n;
		//std::cout << pow(16, j) * n << '\n';
	}
	return res;
}
#else
size_t hextoi(const std::string & number) {
	unsigned int x;
	std::stringstream ss;
	ss << std::hex << number;
	ss >> x;
	return x;
}
#endif
int unpack_dequtes(std::string & buffer) {
	std::string::size_type pos = buffer.find("\"");
	while ( pos != std::string::npos ) {
		std::cout << "unpacking: " << buffer << '\n';
		buffer.erase(pos, 1);
		pos = buffer.find("\"", pos);
		if ( pos == std::string::npos )
			return EXIT_FAILURE;
		buffer.erase(pos, 1);
		pos = buffer.find("\"");
	}
	return EXIT_SUCCESS;
}

void ltrim(std::string & s) {
	s.erase(s.begin(),
			std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

void rtrim(std::string & s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
						 std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

void trim(std::string & s) {
	ltrim(s);
	rtrim(s);
}

void to_lower(std::string & str)
{
//	int len = ;
//	for (int i=0; i < static_cast<int>(str.length()); ++i) {
	for (int i=0; i < str.length(); ++i) {
		str[i] = std::tolower(str[i]);
	}
}


void replace(std::string & str, std::string const & s1, std::string const & s2)
{
	std::size_t found = str.find( s1 );
	while ( found != std::string::npos ) {
		/* std::cout << found << std::endl; */
		str.erase( found, s1.length() );
		str.insert( found, s2 );
		found = str.find( s1 );
	}
}

bool isDigits(const std::string & str) {
	//https://stackoverflow.com/questions/8888748/how-to-check-if-given-c-string-or-char-contains-only-digits
	return str.find_first_not_of("0123456789") == std::string::npos;
}

bool isDigitsAndOws(const std::string & str) {
	return str.find_first_not_of("0123456789 \t") == std::string::npos;
}
