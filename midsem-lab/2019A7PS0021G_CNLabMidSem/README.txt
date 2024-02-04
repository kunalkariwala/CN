31120190021
Steps to run the programs:
	1) gcc server.c -o server (terminal 1)
	2) gcc client.c -o client (terminal 2)
	3) ./server		  (terminal 1)
	4) ./client		  (terminal 2)

Server is by default bound 127.0.0.1. Enter this address when client asks for input.

NOTE: 
While testing the code, if after running the server once, and you get “Cannot bind -1” error while running again, then wait for some time. If you run netstat --tcp --numeric | grep 4444 you will see that the socket on the port 4444 is in TIME_WAIT state. Meaning that connection is closed, so it will be terminated in the timeout specified by the OS. After the timeout expires, the program executes without the error.

