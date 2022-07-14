from os import environ

# Get data from fields

print("Content-type:text/html\r\n\r\n")

sid = None
if environ.get('HTTP_COOKIE'):
    for cookie in (environ['HTTP_COOKIE'].split(';')):
        print(cookie)
        # key, value = cookie.split('=')
        # print(key, value)
        # if key == "id":
        #     sid = value

print("<html>")
print("<head>")
if sid == None:
    print("<title>Welcome!</title>")
else:
    print("<title>Glad you returned!</title>")
print("</head>")
print("<body>")
if sid != None:
    print("<h2>Nice to see you</h2>")
else:
    print("<h2>welome back, your sid is %s</h2>", sid)

print("</body>")
print("</html>")

