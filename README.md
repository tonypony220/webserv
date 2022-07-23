# Webserver
simple HTTP webserver
- handle GET, POST, PUT, DELETE method
- able to handle files
- support CGI scripts written in python, bash or binary

##Warning! 
Do not use it in production due to security not completed.

Behavior of POST method is similar to PUT as it allows to upload files without mime-type headers.

##Run server:
from root of project. 
``` shell
make
./webserv conf
```

##Config params: 

`server name` - name of host as in nginx

`listen` - listening ports

`client_max_body_size` - size in bytes

`root` - directory to search in for route of server or location

`location` - match of URI part after server_name and port 

`enable_session` - directory to look in for route

`error_page` - directory where search error pages. 
**Supported only files like** `404.html`

`allowed_methods` - self expalanatory. If not set - **GET** mehtod

`autoindex` - lists folder with links

`cgi` - enables cgi scripts with provided extension. 

##Sample config 
###Values in square brakets are optional.
```
server {
    server_name serv.org [www.serv.org]
    listen 2001 [2002]

    client_max_body_size 2089600
    root /Users/mehtel/coding/webserv/
    enable_session
    error_page /Users/mehtel/coding/webserv/err_pages
    location / {
        index index.html
        allowed_methods   GET
    }
    location /images/ {

        root /Users/mehtel/coding/webserv/tmp/
        allowed_methods   PUT POST
    }
    location /redir/ {
        redirect http://google.com
    }

    location /put_test/ {
        root              /Users/mehtel/coding/webserv/
        allowed_methods   PUT
    }

    location /upload/ {
        root              /Users/mehtel/coding/webserv/
        allowed_methods   GET PUT POST DELETE
    }

    location /cgi {
        autoindex on
        root /Users/mehtel/coding/webserv/cgi
        cgi sh py
    }
}
```
