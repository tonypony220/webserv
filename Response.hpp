#pragma once
#include <unistd.h>
#include <fstream>
#include "Request.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h> // chmod
#define IN 0
#define OUT 1

#define FILE 2
#define CGI  4
#define GENERIC 8
#define HTML 16
#define UPLOAD 32
#define FILE_ERROR 64

//#define STATE_DISCOVERED  //1
#define STATE_NONE  		  10  //2
#define STATE_WAIT  		  11  //2
#define STATE_INFC_CREATED    12  //3
#define STATE_INFC_PROCESSING 13  //3
#define STATE_DONE  		  14  //3
#define STATE_READY 		  15  //4
#define STATE_ERROR 		  16  //4
#define STATE_WAIT_PROC 	  17  //4

class CgiPipe;

class HttpResponse : public HttpParser {

  public:
//	HttpResponse() : HttpParser() { verbose && std::cout << "HttpResponse created"  << std::endl; }
	HttpResponse(const HttpParser & copy) : HttpParser(copy) {
		verbose && std::cout << "HttpResponse created"  << std::endl;
		init_response();
		server_ptr->match_config();
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
	Server_config		*config;
	std::string			response;
	std::string			response_body; // * temp var for body
	std::vector<BYTE>	response_buffer;
//	std::vector<BYTE>	cgi_resp_buffer;
	std::string			file_type; // * file type for CGI
	int 				fd;	// * for reading file only
	int					pid;
	unsigned int		type;
	unsigned int		resp_state;
	bool				cgi_proc_exited;

	std::vector<BYTE> & get_response_buffer() {
		return response_buffer;
	}
	std::string & get_response_body() {
		return response_body;
	}
	unsigned int & get_resp_state() { return resp_state; }
	unsigned int & get_resp_type() { return type; }

	std::string get_state_type_str() {
		std::string s(YELLOW"state type: ");
		if (type & FILE)    (s.append("FILE "));
		if (type & CGI)     (s.append("CGI "));
		if (type & UPLOAD)  (s.append("UPLOAD "));
		if (type & HTML)    (s.append("HTML "));
		if (type & GENERIC) (s.append("GENERIC "));
		s.append(RESET);
		return s;
	}

	int read_from_file() {
		char buff[BUFF_SIZE];
		memset(buff, 0, BUFF_SIZE);
		int rc = read(fd, buff, BUFF_SIZE - 1);
		if (rc == 0) {
			return END;
		}
		if (rc < 0) {
			log(RED"read file error: ", strerror(errno), RESET);
			return ERROR;
		}
		response_buffer.insert(response_buffer.end(),
							   buff,
							   buff + rc);
		return SUCCESS;
	}
	void fill_buffer_to_send() {
		add_status_line();
		add_headers();
//		response_buffer = response + response_body;
//		log("resp_headers: ", response);
//		log("resp_body: ", response_body);
		response_buffer.insert(response_buffer.end(),
							   response.begin(),
							   response.end()
							   );
		response_buffer.insert(response_buffer.end(),
							   response_body.begin(),
							   response_body.end()
							   );
		response.clear();
		response_body.clear();
	}
	int wait_process() {
		int status;
		int ret = waitpid(pid, &status, WNOHANG);
		std::cout << YELLOW"\rret waitpid=" << ret<< RESET;
		if ( ret != 0
		||  (WIFEXITED(status) && WEXITSTATUS(status) > 0)
		||  WIFSIGNALED(status)) {
			cgi_proc_exited = true;
			return ERROR;
		}
		return SUCCESS;
	}
	int error_page_exists() {
		std::string err_path = 	server_ptr->error_pages_path + "/"
								+ itoa(code) + ".html";
		struct stat s;
		bool exists = stat( err_path.c_str(), &s ) == EXIT_SUCCESS;
		log("error page exists ", err_path, exists);
		if (exists) {
			length = s.st_size;
			open_file_to_read(err_path);
		}
		return exists;
	}

	int ready_to_write() {
		if ( resp_state == STATE_READY ) {
			log(BLUE"response state READY "RESET, get_state_type_str());
			return 1;
		}
		if (!code)
			setCode(HttpStatus::OK);
		if ( code > 300 && !(type & FILE_ERROR) ) {
			log(BLUE"response error READY "RESET, get_state_type_str());
			if ( server_ptr->error_pages_path.size() && error_page_exists()) {
				type = FILE | HTML | FILE_ERROR;
				return 0;
			}
//			} else {
			generate_response_body();
//			}
			fill_buffer_to_send();
			resp_state = STATE_READY;
			return 1;
		}
		if ( type & UPLOAD ) {
			if ( resp_state < STATE_DONE )
				return 0;
			if ( resp_state == STATE_DONE ) {
//				if ( wait_process() == ERROR ) {
//					setCode(HttpStatus::InternalServerError, "cgi fail");
////					response_body.clear();
//					return 0;
//				}
				length = response_body.size();
				fill_buffer_to_send();
				resp_state = STATE_READY;
			}
			return response_buffer.size();
		}
		if ( type & CGI ) {
			if ( resp_state <= STATE_WAIT )
				return 0;
			if ( resp_state == STATE_INFC_CREATED ) {
				fill_buffer_to_send();
				resp_state = STATE_INFC_PROCESSING;
			}

//			setCode(HttpStatus::OK);
//			if ( wait_process() == ERROR && resp_state ) {
//				setCode(HttpStatus::InternalServerError, "cgi fail");
////					response_body.clear();
//				return 0;
//			}

//			log(BLUE"response CGI | UPLOAD "RESET, get_state_type_str());
//			return 1;
//			if ( resp_state < STATE_DONE )
//				return 0;
			wait_process();
			if ( resp_state == STATE_DONE && cgi_proc_exited ) {
				/* state DONE - waiting of pipe. */
				log("<<<<<<<<<<<<<<<<");
				resp_state = STATE_READY;
//				if ( wait_process() == ERROR ) {
//					setCode(HttpStatus::InternalServerError, "cgi fail");
////					response_body.clear();
//					return 0;
//				}
//				length = response_body.size();
//				fill_buffer_to_send();
//				resp_state = STATE_READY;
//				resp_state = STATE_WAIT_PROC;
			}
			return response_buffer.size();
		}
		if ( type & FILE ) {
			log(BLUE"response FILE "RESET, get_state_type_str());
//			if ( !response.empty() )
			if ( resp_state < STATE_WAIT ) {
				fill_buffer_to_send();
				resp_state = STATE_WAIT;
			}
			if ( fd > 0 && read_from_file() != SUCCESS ) {
				log(BLUE"file reading is done"RESET);
				close(fd);
				fd = -1;
				resp_state = STATE_READY;
			}
			return response_buffer.size();
		}
		if ( !response_body.empty() ) {
			log(BLUE"response empty READY "RESET, get_state_type_str());
			fill_buffer_to_send();
			resp_state = STATE_READY;
		}
		log(BLUE"response SIZE "RESET, get_state_type_str());
		return response_buffer.size();
	}

	bool completed() const {
//		log("buffer: ", buffer.size());
//		log("state=", state);
//		if ( (type & CGI) &&  )
//		std::cout << "\r\t\t\t\t\t\t\tresp state=" << resp_state;
		return resp_state == STATE_READY && response_buffer.empty();
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
		  perror("error list dir");
		  return EXIT_FAILURE;
		}
	}

	int search_file() {
		// https://stackoverflow.com/questions/146924/how-can-i-tell-if-a-given-path-is-a-directory-or-a-file-c-c
		std::string path(server_ptr->root + target);
		// realpath does not work
		log("searching in: ", path);

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
					file_type = target.substr(n+1);
					log("file type: ", file_type);
					if (file_type == "html") {
						type = FILE | HTML;
						return EXIT_SUCCESS;
					}
					if ( server_ptr->is_cgi_file_type(file_type) ) {
						log("file will be exec as cgi");
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

	std::string short_log_line() {
		return get_response_status() + " " + itoa(length) + " " + headers["user-agent"];
	}
	std::string get_response_status() {
		return itoa(code) + " " + HttpStatus::reasonPhrase(code);
	}
	void add_status_line() {
		if ( !protocol.size() )
			protocol = "HTTP";
		if ( !version.size() )
			version = "1.1";
		response = protocol + "/" + version + " " + get_response_status() + "\r\n";
	}
	void add_headers() {
		const std::time_t now = std::time(nullptr);
		char buf[64];

		if (strftime(buf, sizeof buf, "%a, %e %b %Y %H:%M:%S GMT", std::gmtime(&now))) {
//			std::cout << std::setw(40) << "    strftime %a %b %e %H:%M:%S %Y" << buf;
			response += "Date: " + std::string(buf) + "\r\n";
		}

//		response += "Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n";
//		size_t body_size = response_body.size();
//		if ( length )
		if ( type != CGI )
			response += "Content-Length: " + itoa(length) +"\r\n";
//		response += "Content-Type: text/html\r\n";
		//response += "Connection: close\r\n";
		response += "Server: tonypony web server\r\n";
		if ( file_type == "html" )
			response += "Content-type: text/html\r\n";
		if ( type != CGI ) {
			response += "\r\n";
		}
//		else {
//			else
//				response += "Content-type: text/plain" + itoa(length) +"\r\n";
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
	int create_file() {
		fd = open(target.c_str(), O_WRONLY | O_NONBLOCK | O_CREAT | O_TRUNC ,  0400);
		if ( fd < 0 ) {
			log(RED"creating file open error: ", strerror(errno),RESET);
			setCode(HttpStatus::NotFound, "open failed");
		}
		return fd;
	}

	int validate_path() {
		get_path_from_target();
		chmod(target.c_str(), S_IRUSR | S_IWUSR);
	//	std::ofstream out( target );
	//	if (!out) {
	//		log(RED"file error: ", strerror(errno),RESET);
	//		return ERROR;
	//	}
	//	out << data;
	//	out.close();
	//	log(GREEN"file saved", RESET);
		return SUCCESS;
	}
	void open_file_to_read(std::string & file) {
		fd = open(file.c_str(), O_RDONLY | O_NONBLOCK);
		if ( fd < 0 ) {
			log(RED"read file open error: ", strerror(errno),RESET);
			setCode(HttpStatus::InternalServerError, "open failed");
		}
	}

	void init_response() {
		// starting response creating
		// this makes internal response.
		// to get start write call get response
		fd = -1; // initialization
		length = 0; // initialization
		type = 0;
		resp_state = STATE_NONE;
		cgi_proc_exited = false;
		if ( !code ) {
			if ( (method == "GET" || method == "DELETE")  && search_file() == EXIT_FAILURE ) {
				setCode(HttpStatus::NotFound, "file not found");
			} else if ( method == "GET" && type & CGI ) {
//				get_path_from_target();
			}
			else if ( method == "GET" && ( type & FILE ))
			{
				get_path_from_target();
				open_file_to_read(target);
//				fd = open(target.c_str(), O_RDONLY | O_NONBLOCK);
//				if ( fd < 0 ) {
//					log(RED"read file open error: ", strerror(errno),RESET);
//					setCode(HttpStatus::InternalServerError, "open failed");
//				}
			}
			else if ( method == "DELETE" ) { // TODO
				// unlink
				get_path_from_target();
				int ret = unlink(target.c_str());
				if ( ret != 0 )
					setCode(HttpStatus::InternalServerError, "delete failed");
				else {
					setCode(HttpStatus::NoContent);
					fill_buffer_to_send();
					resp_state = STATE_READY;
				}
			}
			else if (method == "POST" || method == "PUT")
			{ // TODO
				type |= UPLOAD;
				if (validate_path() == SUCCESS) {
					setCode(HttpStatus::NoContent);
				}
//				fill_buffer_to_send();
//				state = STATE_READY;
			}
			else
			{
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

	bool does_need_interface() {
		if ( (type & (CGI | UPLOAD)) && resp_state == STATE_NONE ) {
			resp_state = STATE_WAIT;
//			log(BLUE"need io? true "RESET, get_state_type_str());
			return true;
		}
//		log(BLUE"need io? false"RESET, get_state_type_str());
		return false;
	}

//	int spawn_process(const char *const *args) {
	int spawn_process(std::vector<std::string> string_args) {
		/* Create copy of current process */

		/* possible to get c_strs only in scope to keep them valid */
//		set_environ_for_cgi();
		std::vector<const char *> args;
//		log(YELLOW"cgi args: "RESET);
		for (int i=0; i<string_args.size(); i++) {
//			log("\t*", string_args[i]);
			args.push_back(string_args[i].c_str());
		}
		args.push_back(0);

		int pid = fork();
		/* The parent`s new pid will be 0 */
		if (pid == 0) {
			/* We are now in a child progress
			Execute different process */
//			execvpe(args[0], (char* const*)args, (char* const*)pEnv);
//			execvp(args[0], (char* const*)&args[0]); //(char* const*)
			execvp(args[0], (char* const*)&args[0]); //(char* const*)
			/* This code will never be executed */
			std::cerr << RED"\t\t\t\t\t<<<<<<fork error<<<<<<<" << strerror(errno) << RESET;
			exit(EXIT_FAILURE);
		}
		/* We are still in the original process */
		return pid;
	}
	void set_environ_for_cgi() {
		log(YELLOW"setting environ"RESET);
		setenv("QUERY_STRING", query_string.c_str(), 1);
		setenv("REQUEST_METHOD", method.c_str(), 1);
		setenv("PATH_INFO", (server_ptr->root + target).c_str(), 1);
//		if (const char* env_p = std::getenv("QUERY_STRING"))
//			std::cout << "Your QUERY_STRING is: " << env_p << '\n';
//		else
//			std::cout << "no QUERY_STRING is: "  << '\n';

	}
	std::vector<std::string> create_cgi_args() {
		std::vector<std::string> args;
		std::string path(server_ptr->root + target);
//		std::string			executable;
		if (file_type == "py") {
			args.push_back("python3");
			args.push_back(path);
		}
		else if (file_type == "cgi" || file_type == "sh")
			args.push_back("./" + path);
//			executable = "./" + target;

//		std::string exec_name("python3");
//		args.push_back(exec_name);
//		args.push_back(path);
//		args.push_back(exec_name.c_str());
//		for (int i=0;i<9;i++) {log(int(exec_name.c_str()[i]), " ", exec_name.c_str()[i]);}
//		args.push_back(path.c_str());
//		args.push_back(0);
//		log("cgi args: ");
//		log("\t", args[0]);
//		log("\t", args[1]);
		return args;
	}

	int cgi() {
		//Формируем в глобальных переменных тело запроса и его длинну
//		const std::string strRequestBody = "===this is request body===\n";
//		const std::string strRequestHeader = "Content-Length="
//				+ std::to_string((long long)strRequestBody.length());
//		std::vector<const char *> args;
//		std::string path(server_ptr->root + target);
//		const char *pszChildProcessArgs[3] = {"python3",
//											  path.c_str(),
		set_environ_for_cgi();
//											  0};
		int fdin[2], fdout[2];
		//fdin[0] = fdin[1] = fdout[0] = fdout[1] = -1;

		if (pipe(fdin) != 0 || pipe(fdout) != 0) {
			std::cerr << "Cannot create CGI pipe";
			setCode(HttpStatus::InternalServerError, "cgi fail");
			return ERROR;
		}

		// Duplicate stdin and stdout file descriptors
		int fdOldStdIn = dup(fileno(stdin));
		int fdOldStdOut = dup(fileno(stdout));

		// Duplicate end of pipe to stdout and stdin file descriptors
		if ((dup2(fdout[OUT], fileno(stdout)) == -1) || (dup2(fdin[IN], fileno(stdin)) == -1)) {
			setCode(HttpStatus::InternalServerError, "cgi fail");
			return ERROR;
		}
		// Close original end of pipe
		close(fdin[IN]);
		close(fdout[OUT]);

		pid = spawn_process(create_cgi_args());
		if ( pid < 0 ) {
			setCode(HttpStatus::InternalServerError, "cgi fail");
			return ERROR;
		}

		// Duplicate copy of original stdin an stdout back into stdout
		dup2(fdOldStdIn, fileno(stdin));
		dup2(fdOldStdOut, fileno(stdout));

		// Close duplicate copy of original stdin and stdout
		close(fdOldStdIn);
		close(fdOldStdOut);

		//Отдаем тело запроса дочернему процессу
//		write(fdin[OUT], &request_buffer[0], request_buffer.size());
		resp_state = STATE_INFC_CREATED;
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

