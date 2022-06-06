#pragma once
#include <unistd.h>
#include "Request.hpp"
//#include "tcpSession.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define IN 0
#define OUT 1

class CgiPipe;

class HttpResponse : public HttpParser {

  public:
//	HttpResponse() : HttpParser() { verbose && std::cout << "HttpResponse created"  << std::endl; }
	HttpResponse(const HttpParser & copy) : HttpParser(copy) { verbose && std::cout << "HttpResponse created"  << std::endl; }
	HttpResponse( const HttpResponse & copy ) : HttpParser(copy) { *this = copy; }
	~HttpResponse( void ) { verbose && std::cout << "HttpResponse destructed, fd: "  << std::endl; }
///		HttpResponse & operator=( const HttpResponse & other ) {
///			fd = other.getFd();
///			request = other.getRequest();
///			response = other.getResponse();
///			return *this;
///		}

	std::string   response;
	std::string	  response_body;
	std::string   file_type;

	std::string get_response() {
		std::string res( response + response_body );
		response.clear();
		return res;
	}

	std::string get_host() { // TODO
		return std::string(headers["Host"]);
	}

	int  list_dir(std::string & path) {
		//std::vector<std::string> lst;
		DIR						*dir;
		struct dirent			*ent;
		std::string				host(get_host());

		if ((dir = opendir (path.c_str())) != NULL) {
		  /* print all the files and directories within directory */
		  while ((ent = readdir (dir)) != NULL) {
			// TODO add href html links
			response_body +=  host + target + std::string(ent->d_name) + "\n";
		  }
		  closedir (dir);
		  return EXIT_SUCCESS;
		} else {
		  /* could not open directory */
		  perror ("");
		  return EXIT_FAILURE;
		}
	}

	int search_file() {
		// https://stackoverflow.com/questions/146924/how-can-i-tell-if-a-given-path-is-a-directory-or-a-file-c-c
		std::string path(server_ptr->root + target);	
		struct stat s;
		if( stat( path.c_str(), &s ) == EXIT_SUCCESS )
		{
		    if( s.st_mode & S_IFDIR && server_ptr->dir_listing) {
				// TODO search for index.html?
				list_dir( path );
				return EXIT_SUCCESS;
		    }
		    else if( s.st_mode & S_IFREG )
		    { //it's a file -> determine_file_type
				std::string::size_type n = target.rfind('.');
				if (n != std::string::npos) {
					file_type = target.substr(n);
					log("file type: ", file_type);
					if (file_type == "html")
						return EXIT_SUCCESS;
					if (file_type == server_ptr->cgi_extension) {
						return EXIT_SUCCESS;
				}
				log("file type not supported: ", path);
				return EXIT_FAILURE;
				}
				log("file nor reg nor dir: ", path);
				return EXIT_FAILURE;
			}
			else
			{
				log("not able open file: ", path, strerror(errno));
				return EXIT_FAILURE;
				//error
			}
		}
		log("error stat", path, strerror(errno));
		return EXIT_FAILURE;
	}
	std::string get_response_status() {
		return itoa(code) + " " + HttpStatus::reasonPhrase(code);
	}
	void add_status_line() {
		response = protocol + "/" + version + " " + get_response_status() + "\r\n";
	}
	void add_headers() {
		response += "Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n";
		size_t body_size = response_body.size();
		if ( body_size ) 
			response += "Content-Length: " + itoa(body_size) +"\r\n";
		response += "Content-Type: text/html\r\n";
		//response += "Connection: close\r\n";
		response += "Server: tonypony web server\r\n";
		response += "\r\n";
	}
	void make_response_body() {
		if (!code) 
			setCode(HttpStatus::OK);
		if ( code / 100 > 3 ) {
			response_body = 
			"<html>\n"
			"<head><title>"+get_response_status()+"</title></head>\n"
			"<body>\n"
			"<center><h1>" + get_response_status() + "</h1></center>\n"
			"<hr><center>web server</center>\n"
			"<hr><center>" + debug_info + "</center>\n"
			"</body>\n" 
			"</html>\n";
		} else {
			response_body = "Hello World! My payload includes a trailing\r\n";
		}
	}

	void make_response() {
		if (!code && method == "GET" && search_file() == EXIT_FAILURE) // TODO different files type 
			setCode(HttpStatus::NotFound, "file not found");
		make_response_body();
		add_status_line();
		add_headers();
		//response = "HTTP/1.1 200 OK\r\n"
     	//		   "Date: Mon, 27 Jul 2009 12:28:53 GMT\r\nContent-Length: 45\r\n\r\nHello World! My payload includes a trailing\r\n";
	}
	int spawn_process(const char *const *args, const char * const *pEnv){
		/* Create copy of current process */
		int pid = fork();
		/* The parent`s new pid will be 0 */
		if(pid == 0) {
			/* We are now in a child progress 
			Execute different process */
			execve(args[0], (char* const*)args, (char* const*)pEnv);
			/* This code will never be executed */
			exit(EXIT_SUCCESS);
		}
		/* We are still in the original process */
		return pid;
	}

	bool is_cgi() {
		return file_type.size() && (file_type == server_ptr->cgi_extension);
	}

	int cgi() {
		//Формируем в глобальных переменных тело запроса и его длинну
		const std::string strRequestBody = "===this is request body===\n";
		const std::string strRequestHeader = "Content-Length=" + std::to_string((long long)strRequestBody.length());

//Формируем переменные окружения которые будут отосланы дочернему процессу
		const char *pszChildProcessEnvVar[4] = {strRequestHeader.c_str(), "VARIABLE2=2", "VARIABLE3=3", 0};

//Формируем переменные командной строки для дочернего процесса. Первая переменная - путь к дочернему процессу.
		const char *pszChildProcessArgs[4] = {"./Main_Child.exe", "first argument", "second argument", 0};
		int fdin[2], fdout[2];
		//fdin[0] = fdin[1] = fdout[0] = fdout[1] = -1;

		if (pipe(fdin) != 0 || pipe(fdout) != 0) {
			std::cerr << "Cannot create CGI pipe";
			return ERROR;
		}

		// Duplicate stdin and stdout file descriptors
		int fdOldStdIn = dup(fileno(stdin));
		int fdOldStdOut = dup(fileno(stdout));

		// Duplicate end of pipe to stdout and stdin file descriptors
		if ((dup2(fdout[OUT], fileno(stdout)) == -1) || (dup2(fdin[IN], fileno(stdin)) == -1))
			return ERROR;
		// Close original end of pipe
		close(fdin[IN]);
		close(fdout[OUT]);

		const int nChildProcessID = spawn_process(pszChildProcessArgs, pszChildProcessEnvVar);

		// Duplicate copy of original stdin an stdout back into stdout
		dup2(fdOldStdIn, fileno(stdin));
		dup2(fdOldStdOut, fileno(stdout));

		// Close duplicate copy of original stdin and stdout
		close(fdOldStdIn);
		close(fdOldStdOut);

		//Отдаем тело запроса дочернему процессу
		write(fdin[OUT], strRequestBody.c_str(), strRequestBody.length());
		return fdout[IN];

	//	while (1)
	//	{
	//		//Читаем ответ от дочернего процесса
	//		char bufferOut[100000];
	//		int n = read(fdout[IN], bufferOut, 100000);
	//		if (n > 0)
	//		{
	//			//Выводим ответ на экран
	//			fwrite(bufferOut, 1, n, stdout);
	//			fflush(stdout);
	//		}
	//		//Если дочерний процесс завершился, то завершаем и родительский процесс
	//		int status;
	//		if (waitpid(nChildProcessID, &status, WNOHANG) > 0)
	//			break;
	//	}
		//return SUCCESS;
	}
	
	bool ready() const { return true; }  //todo
	bool sent() const { return response.empty(); }  //todo
};

