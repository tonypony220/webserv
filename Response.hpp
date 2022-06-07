#pragma once
#include <unistd.h>
#include <fstream>
#include "Request.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h> // chmod
#define IN 0
#define OUT 1

#define FILE 2
#define CGI  4
#define GENERIC 8
#define HTML 16

//#define STATE_DISCOVERED  //1
#define STATE_NONE  0  //2
#define STATE_WAIT  1  //2
#define STATE_DONE  2  //3
#define STATE_READY 3  //4
#define STATE_ERROR 4  //4

#define CGI_STATE_INIT 	 1
#define CGI_STATE_CALLED 2
#define CGI_STATE_DONE   3
#define CGI_STATE_ERROR  4

class CgiPipe;

class HttpResponse : public HttpParser {

  public:
//	HttpResponse() : HttpParser() { verbose && std::cout << "HttpResponse created"  << std::endl; }
	HttpResponse(const HttpParser & copy) : HttpParser(copy) {
		verbose && std::cout << "HttpResponse created"  << std::endl;
		init_response();
	}
	HttpResponse( const HttpResponse & copy ) : HttpParser(copy) { *this = copy; }
	~HttpResponse( void ) { verbose && std::cout << "HttpResponse destructed, fd: "  << std::endl; }
///		HttpResponse & operator=( const HttpResponse & other ) {
///			fd = other.getFd();
///			request = other.getRequest();
///			response = other.getResponse();
///			return *this;
///		}
	// 1. store headers.
	// 2. store ready full response text
	std::string		response;
	std::string		response_body; // * temp var for body
	std::string		response_buffer;
	std::string		file_type; // * file type for CGI
	int 			fd;	// * for reading file only
	int				type;
	int				state;

	std::string & get_buffer() {
		return response_buffer;
	}

	int read_from_file() {
		char buff[BUFF_SIZE];
		memset(buff, 0, BUFF_SIZE);
		int rc = read(fd, buff, BUFF_SIZE - 1);
		if (rc == 0) {
			return END;
		}
		if (rc < 0) {
			log(RED"read file error: ", strerror(errno),RESET);
			return ERROR;
		}
		response_buffer.append( buff );
		return SUCCESS;
	}
	void fill_buffer_to_send() {
		add_status_line();
		add_headers();
		response_buffer = response + response_body;
		response.clear();
		response_body.clear();
	}
	int ready_to_write() {
//		log("state=", state);
		if ( state == STATE_READY )
			return 1;
		if (!code)
			setCode(HttpStatus::OK);
		if ( code > 300 ) {
			generate_response_body();
			fill_buffer_to_send();
			state = STATE_READY;
			return 1;
		}
		if ( type & CGI ) {
			if ( state < STATE_DONE )
				return 0;
			if ( state == STATE_DONE ) {
				fill_buffer_to_send();
				state = STATE_READY;
			}
			return 1;
		}
		if ( type & FILE ) {
//			if ( !response.empty() )
			if ( state < STATE_WAIT ) {
				fill_buffer_to_send();
				state = STATE_WAIT;
			}
			if ( fd > 0 && read_from_file() != SUCCESS ) {
				log(BLUE"file reading is done"RESET);
				close(fd);
				fd = -1;
				state = STATE_READY;
			}
			return response_buffer.size();
		}
		if ( !response_body.empty() ) {
			fill_buffer_to_send();
			state = STATE_READY;
		}
		return response_buffer.size();
	}

	bool completed() const {
//		log("buffer: ", buffer.size());
//		log("state=", state);
		return state >= STATE_READY && response_buffer.empty();
//		if ( cgi_state >= CGI_STATE_DONE )
//			return true;
//		if ( file_state ) {
//			if ( fd < 0 )
//				return true;
//			return false;
//		}
//		return buffer.empty();
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
				type = GENERIC;
				list_dir( path );
				length = response_body.size();
				return EXIT_SUCCESS;
		    }
		    else if( s.st_mode & S_IFREG )
		    { //it's a file -> determine_file_type
				length = s.st_size;
				std::string::size_type n = target.rfind('.');
				if (n != std::string::npos) {
					file_type = target.substr(n);
					log("file type: ", file_type);
					if (file_type == "html") {
						type = FILE | HTML;
						return EXIT_SUCCESS;
					}
					if (file_type == server_ptr->cgi_extension) {
						type = CGI;
						return EXIT_SUCCESS;
					}
				}
				type = FILE;
//					log(RED"file type not supported: ", path, RESET);
//					return EXIT_FAILURE;
//				} else {
//					log(RED"file nor reg nor dir: ", path, RESET);
//					return EXIT_FAILURE;
//				}
				return EXIT_SUCCESS;
			} else {
				log(RED"not able open file: ", path, strerror(errno), RESET);
				return EXIT_FAILURE;
				//error
			}
		}
		log(RED"error stat:", path, strerror(errno), RESET);
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
//		size_t body_size = response_body.size();
		if ( length )
			response += "Content-Length: " + itoa(length) +"\r\n";
		response += "Content-Type: text/html\r\n";
		//response += "Connection: close\r\n";
		response += "Server: tonypony web server\r\n";
		response += "\r\n";
	}
	void generate_response_body() {
//		if (!code)
//			setCode(HttpStatus::OK);
//		if ( code / 100 > 3 ) {
		response_body =
			"<html>\n"
			"<head><title>"+get_response_status()+"</title></head>\n"
			"<body>\n"
			"<center><h1>" + get_response_status() + "</h1></center>\n"
			"<hr><center>web server</center>\n"
			"<hr><center>" + debug_info + "</center>\n"
			"</body>\n"
			"</html>\n";
		length = response_body.size();
//		}
	}
	void get_path_from_target() {
		target.erase(0, 1);
	}

	int write_to_file(std::string & data) {
		get_path_from_target();
		chmod(target.c_str(), S_IRUSR | S_IWUSR);
		std::ofstream out( target );
		if (!out) {
			log(RED"file error: ", strerror(errno),RESET);
			return ERROR;
		}
		out << data;
		out.close();
		log(GREEN"file saved", RESET);
		return SUCCESS;
	}

	void init_response() {
		// starting response creating
		// this makes internal response.
		// to get start write call get response
		fd = -1; // initialization
		length = 0; // initialization
		type = 0;
		state = STATE_NONE;
		if ( !code ) {
			if ( (method == "GET" || method == "DELETE")  && search_file() == EXIT_FAILURE ) {
				setCode(HttpStatus::NotFound, "file not found");
			} else if ( method == "GET" && type == CGI ) {

			} else if ( method == "GET" && ( type & FILE )) {
				get_path_from_target();
				fd = open(target.c_str(), O_RDONLY | O_NONBLOCK);
				if ( fd < 0 ) {
					log(RED"file open error: ", strerror(errno),RESET);
					setCode(HttpStatus::InternalServerError, "open failed");
				}
			} else if ( method == "DELETE" ) { // TODO
				// unlink
			} else if (method == "POST" || method == "PUT") { // TODO
				if (write_to_file(buffer) == SUCCESS)
					setCode(HttpStatus::NoContent);
				fill_buffer_to_send();
				state = STATE_READY;
			} else {
				setCode(HttpStatus::InternalServerError, "what?");
			}
		}
//		if (!code && method == "GET" && search_file() == EXIT_FAILURE) // TODO different files type
//			setCode(HttpStatus::NotFound, "file not found");
//		if (!code && (method == "POST" || method == "PUT")) {
//			if ( write_to_file(target, buffer) == SUCCESS )
//				setCode(HttpStatus::NoContent);
//		}

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
		if ( (type & CGI) && state == STATE_NONE )	{
			state = STATE_WAIT;
			return true;
		}
		return false;
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
	
};

