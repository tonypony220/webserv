from os import environ

# Get data from fields

print("Content-type:text/html\r\n\r\n")

sid = None
if environ.get('HTTP_COOKIE'):
    for cookie in (environ['HTTP_COOKIE'].split(';')):
        if cookie:
            # print(cookie)
            key, value = cookie.split('=')
            # print(cookie.split('='))
            # print(key, value)
            if key == "id":
                sid = value

print("<html>")
print("<head>")
if not sid:
    print("<title>Welcome!</title>")
else:
    print("<title>Glad you returned!</title>")
print("</head>")
print("<body>")
if not sid:
    print("<h2>Nice to see you</h2>")
else:
    print(f"<h2>welcome back, your SESSION ID is <code>{sid}</code></h2>")

print("</body>")
print("</html>")

