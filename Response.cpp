#include "Response.hpp"
#include "Utils.hpp"

//HttpResponse::HttpResponse() : HttpParser() {
//	verbose && std::cout << "HttpResponse created"  << std::endl;
//}

HttpResponse::HttpResponse(const HttpParser & copy) : HttpParser(copy) {
//		verbose && std::cout << "HttpResponse created"  << std::endl;
		init_response();
	}
HttpResponse::HttpResponse( const HttpResponse & copy ) : HttpParser(copy) {
//		verbose && std::cout << PURPLE"HttpResponse copied, fd: "RESET << std::endl;
		*this = copy;
	}
HttpResponse::~HttpResponse( void ) {
//		verbose && std::cout << "HttpResponse destructed, fd: " << std::endl;
	}

///		HttpResponse & operator=( const HttpResponse & other ) {
///			fd = other.getFd();
///			request = other.getRequest();
///			response = other.getResponse();
///			return *this;
///		}
	// 1. store headers.
	// 2. store ready full response text
	std::vector<BYTE> & HttpResponse::get_response_buffer() {
		return response_buffer;
	}
	std::string & HttpResponse::get_response_body() {
		return response_body;
	}
	unsigned int & HttpResponse::get_resp_state() { return resp_state; }
	unsigned int & HttpResponse::get_resp_type() { return type; }

	std::string HttpResponse::get_state_type_str() {
		std::string s(YELLOW"state type: ");
		if (type & FILE)    (s.append("FILE "));
		if (type & CGI)     (s.append("CGI "));
		if (type & UPLOAD)  (s.append("UPLOAD "));
		if (type & HTML)    (s.append("HTML "));
		if (type & GENERIC) (s.append("GENERIC "));
		s.append(RESET);
		return s;
	}

	int HttpResponse::read_from_file() {
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
	void HttpResponse::fill_buffer_to_send() {
		add_status_line();
		add_headers();
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
	int HttpResponse::wait_process() {
		int status;
		int ret = waitpid(pid, &status, WNOHANG);
		std::cout << YELLOW"\rret of pid=" << pid << " waitpid=" << ret<< RESET;
		if ( ret != 0
		||  (WIFEXITED(status) && WEXITSTATUS(status) > 0)
		||  WIFSIGNALED(status)) {
			cgi_proc_exited = true;
			return ERROR;
		}
		return SUCCESS;
	}
	int HttpResponse::error_page_exists() {
		std::string err_path = config->error_pages_path + "/"
								+ itoa(code) + ".html";
		clear_path(err_path);
		struct stat s;
		bool exists = stat( err_path.c_str(), &s ) == EXIT_SUCCESS;
		log("error page exists ", exists);
		if (exists) {
			length = s.st_size;
			open_file_to_read(err_path);
		}
		return exists;
	}

	int HttpResponse::ready_to_write() {
//		log("\r"BLUE"current state="RESET, resp_state, get_state_type_str());
		if ( resp_state == STATE_READY ) {
//			log(BLUE"response state READY "RESET, get_state_type_str());
			return 1;
		}
		if (!code)
			setCode(HttpStatus::OK);
		if ( code > 300 && !(type & FILE_ERROR) ) {
//			log("type=",type,  get_state_type_str());
//			if (type & RESPONSE_TIMEOUT_ERROR) {
//				/* make response ready so it able to */
//				resp_state = STATE_READY;
//				response_buffer.clear();
//				return 1;
//			}
			resp_state = STATE_NONE;
			if ( config->error_pages_path.size() && error_page_exists()) {
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
			/* in case of timeout should wait file
			 * till it change resp_state -> DONE */
			if ( resp_state < STATE_DONE )
				return 0;
			if ( resp_state == STATE_DONE ) {
				close(fd);
				length = response_body.size();
				fill_buffer_to_send();
				resp_state = STATE_READY;
			}
			return response_buffer.size();
		}
		if ( type & CGI ) {
			/* in case of timeout should wait pipe
			 * till it change resp_state -> DONE */
			if ( resp_state <= STATE_WAIT )
				return 0;
			if ( resp_state == STATE_INFC_CREATED ) {
				fill_buffer_to_send();
				resp_state = STATE_INFC_PROCESSING;
			}
			wait_process();
			if ( resp_state == STATE_DONE && cgi_proc_exited ) {
				/* resp_state STATE_DONE -> waiting of pipe. */
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
//			if ( !response.empty() )
			if ( resp_state < STATE_WAIT ) {
				fill_buffer_to_send();
				resp_state = STATE_WAIT;
			}
			if ( fd > 0 && read_from_file() != SUCCESS ) {
				int ret = close(fd);
				std::stringstream ss;
				ss<<BLUE"file reading is done. closed="<<!ret<<" fd="<<fd<<RESET;
				log(ss.str());
				fd = -1;
				resp_state = STATE_READY;
			}
			return response_buffer.size();
		}
		if ( !response_body.empty() ) {
//			log(BLUE"response empty READY "RESET, get_state_type_str());
			fill_buffer_to_send();
			resp_state = STATE_READY;
		}
//		log(BLUE"response SIZE "RESET, get_state_type_str());
		return response_buffer.size();
	}
	void  HttpResponse::abort() {
		log(RED"aborting"RESET);
		type |= RESPONSE_TIMEOUT_ERROR;
		if ( (type & CGI) && pid ) {
			log(RED"killing ", pid, RESET);
			kill(pid, SIGKILL);
		}
		if (fd > -1)
			close(fd);
//		if (type & FILE)
//			resp_state = STATE_READY;

	}
	bool  HttpResponse::been_sent() {
		return !(type & RESPONSE_TIMEOUT_ERROR);
	}

	bool  HttpResponse::completed() const {
		bool r = (resp_state == STATE_READY &&
				(response_buffer.empty() ||  type & RESPONSE_TIMEOUT_ERROR));
//		if (r && fd > -1 ) close(fd);
		return r;
				//; //resp_state == STATE_DONE ;
//		if ( cgi_state >= CGI_STATE_DONE )
//			return true;
//		if ( file_state ) {
//			if ( fd < 0 )
//				return true;
//			return false;
//		}
//		return buffer.empty();
	}

	std::string  HttpResponse::get_host() { // TODO
		return std::string(headers["Host"]);
	}
//	<html>
//	<head><title>Index of /yo/</title></head>
//	<body>
//	<h1>Index of /yo/</h1><hr><pre><a href="../">../</a>
//	<a href="Yeah/">Yeah/</a>                                              23-Jun-2022 13:43                   -
//	<a href="nop/">nop/</a>                                               23-Jun-2022 13:43                   -
//	<a href="youpi.bad_extension">youpi.bad_extension</a>                                23-Jun-2022 13:42                   0
//	</pre><hr></body>
//	</html>

	int  HttpResponse::autoindex_directory(std::string & path) {
//		DIR						*dir;
//		struct dirent			*ent;
//		std::string				host(get_host());

		std::vector<std::string> listing = list_dir(path);
		response_body += "<html>\n"
						 "<head><title>Index of /yo/</title></head>\n"
						 "<body>\n"
						 "<h1>Index of /yo/</h1><hr><pre><a href=\"../\">../</a>\n";

		for (size_t i=0; i < listing.size(); i++ ) {
			response_body += "<a href=\"" + target + "/" +  listing[i] + "\">" + listing[i] +"</a>\n";
		}
		response_body += "</pre><hr></body>\n"
						 "<html>\n";
	}

	int  HttpResponse::search_file() {
	// https://stackoverflow.com/questions/146924/how-can-i-tell-if-a-given-path-is-a-directory-or-a-file-c-c
	// realpath does not work
		log("searching in: ", path);

		struct stat s;
		std::string result = path;
		if( stat( path.c_str(), &s ) == EXIT_SUCCESS )
		{
			log("search -> listing dir", path);
		    if( s.st_mode & S_IFDIR && location->dir_listing) {
				type = GENERIC;
				autoindex_directory( path );
				length = response_body.size();
				return EXIT_SUCCESS;
//		    } else if (  ) {
//
			} else if ( s.st_mode & S_IFREG || (s.st_mode & S_IFDIR
				&& find_file(location->filenames, path, result))) {
				log("search -> file ", path);
				//it's a file -> determine_file_type
				path = result;
				length = get_file_size(path);
				std::string::size_type n = target.rfind('.');
				if (n != std::string::npos) {
					file_type = target.substr(n+1);
					log("file type: ", file_type);
					if (file_type == "html") {
						type = FILE | HTML;
						return EXIT_SUCCESS;
					}
					if ( location->is_cgi_file_type(file_type) ) {
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

	std::string  HttpResponse::short_log_line() {
		return get_response_status() + " " + itoa(length) + " " + headers["user-agent"];
	}
	std::string  HttpResponse::get_response_status() {
		return itoa(code) + " " + HttpStatus::reasonPhrase(code);
	}
	void  HttpResponse::add_status_line() {
		if ( !protocol.size() )
			protocol = "HTTP";
		if ( !version.size() )
			version = "1.1";
		response = protocol + "/" + version + " " + get_response_status() + "\r\n";
	}
	void  HttpResponse::add_headers() {
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
		if (code == HttpStatus::TemporaryRedirect)
			response += "Location: " + location->redirect_uri +"\r\n";
//		response += "Content-Type: text/html\r\n";
		//response += "Connection: close\r\n";
		response += "Server: "+server_ptr->app_name+"\r\n";
		if (config->enable_session && session_id.empty()) {
			response += "Set-Cookie: id="+ server_ptr->generate_session_id() +"\r\n";
		}
		if ( file_type == "html" )
			response += "Content-type: text/html\r\n";
		if ( type != CGI ) {
			response += "\r\n";
		}
//		else {
//			else
//				response += "Content-type: text/plain" + itoa(length) +"\r\n";
	}

	void  HttpResponse::generate_response_body() {
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
	int  HttpResponse::create_file() {
		fd = open(path.c_str(), O_WRONLY | O_NONBLOCK | O_CREAT | O_TRUNC ,  0400);
		if ( fd < 0 ) {
			log(RED"creating file open error: ", strerror(errno),RESET);
			setCode(HttpStatus::NotFound, "open failed");
		}
		return fd;
	}

	int  HttpResponse::validate_path() {
//		get_path_from_target();
		chmod(path.c_str(), S_IRUSR | S_IWUSR);
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
	void  HttpResponse::open_file_to_read(std::string & file) {
		fd = open(file.c_str(), O_RDONLY | O_NONBLOCK);
		if ( fd < 0 ) {
			log(RED"read file open error: ", strerror(errno),RESET);
			setCode(HttpStatus::InternalServerError, "open failed");
		}
		else
			log(PURPLE"--opened file fd=", fd, RESET);
	}

	void  HttpResponse::init_response() {
		// starting response creating
		// this makes internal re.
		// to get start write call get response
		fd = -1; // initialization
		length = 0; // initialization
		type = 0;
		pid = 0;
		resp_state = STATE_NONE;
		cgi_proc_exited = false;

		// must search config even if code has been set cause error path
//		match_config_and_location();
		if ( !code ) {

			if ( (method == "GET" || method == "DELETE")
			&& search_file() == EXIT_FAILURE ) {
				setCode(HttpStatus::NotFound, "file not found");
			} else if ( method == "GET" && type & CGI ) {
				log("type is CGI");
//				get_path_from_target();
			} else if ( method == "GET" && ( type & FILE )) {
//				get_path_from_target();
				open_file_to_read(path);
//				fd = open(target.c_str(), O_RDONLY | O_NONBLOCK);
//				if ( fd < 0 ) {
//					log(RED"read file open error: ", strerror(errno),RESET);
//					setCode(HttpStatus::InternalServerError, "open failed");
//				}
			} else if ( method == "DELETE" ) { // TODO
				// unlink
//				get_path_from_target();
				int ret = unlink(path.c_str());
				if ( ret != 0 ) {
					std::stringstream s;
					s << "delete failed: " << strerror(errno);
					setCode(HttpStatus::InternalServerError, s.str());
				} else {
					setCode(HttpStatus::NoContent);
					fill_buffer_to_send();
					resp_state = STATE_READY;
				}
			} else if (method == "POST" || method == "PUT") { // TODO
				type |= UPLOAD;
				if (validate_path() == SUCCESS) {
					setCode(HttpStatus::NoContent);
				}
//				fill_buffer_to_send();
//				state = STATE_READY;
			}
//			else {
//				setCode(HttpStatus::InternalServerError, "what?");
//			}
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

	bool  HttpResponse::does_need_interface() {
		if ( (type & (CGI | UPLOAD)) && resp_state == STATE_NONE ) {
			resp_state = STATE_WAIT;
//			log(BLUE"need io? true "RESET, get_state_type_str());
			return true;
		}
//		log(BLUE"need io? false"RESET, get_state_type_str());
		return false;
	}

//	int spawn_process(const char *const *args) {
	int  HttpResponse::spawn_process(std::vector<std::string> string_args) {
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

		pid = fork();
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
	void  HttpResponse::set_environ_for_cgi() {
		log(YELLOW"setting environ"RESET);
		setenv("QUERY_STRING", query_string.c_str(), 1);
		setenv("REQUEST_METHOD", method.c_str(), 1);
		setenv("PATH_INFO", (path).c_str(), 1);
		if (config->enable_session) { // && !cookies.empty()) {
			if (!cookies.empty()) {
				std::stringstream ss;
				headerItor it = cookies.begin();
				for (; it != cookies.end(); it++)
					ss << it->first << "=" << it->second << ";";
				setenv("HTTP_COOKIE", ss.str().c_str(), 1);
			} else {
				unsetenv("HTTP_COOKIE");
			}
		}

//		if (const char* env_p = std::getenv("QUERY_STRING"))
//			std::cout << "Your QUERY_STRING is: " << env_p << '\n';
//		else
//			std::cout << "no QUERY_STRING is: "  << '\n';

	}
	std::vector<std::string>  HttpResponse::create_cgi_args() {
		std::vector<std::string> args;
//		std::string path(target);
//		std::string			executable;
		if (file_type == "py") {
			args.push_back("python3");
			args.push_back(path);
		}
		else if (file_type == "cgi")
			args.push_back("./" + path);
		else if (file_type == "sh") {
			args.push_back("sh");
			args.push_back(path);
		}
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

	int  HttpResponse::cgi() {
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
		fcntl(fdin[IN], F_SETFL, O_NONBLOCK);
		fcntl(fdin[OUT], F_SETFL, O_NONBLOCK);
		fcntl(fdout[IN], F_SETFL, O_NONBLOCK);
		fcntl(fdout[OUT], F_SETFL, O_NONBLOCK);
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

		// this
		int w = write(fdin[OUT], &request_buffer[0], request_buffer.size());
		while ( w > 0) {
			request_buffer.erase(request_buffer.begin(), request_buffer.begin() + w + 1);
			w = write(fdin[OUT], &request_buffer[0], request_buffer.size());
		}
		if (w < 0)
			perror(RED"write pipe error"RESET);
		close(fdin[OUT]);
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
	

