import subprocess
a = "siege -c 25 -r 1 http://localhost:2001/"
subprocess.run(a.split(" "))
