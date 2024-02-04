Arun Ganti
2019A7PS0021G

Steps to run the files:
	$ gcc server.c -o server
	$ gcc client.c -o client

All interaction takes place on the client side.

Enter a name that starts with a captial letter to chat with the server
Enter only '.' to terminate the chat

NOTE: While testing the code, if after running the server once, and you get “Cannot bind -1” error while running again, then wait for some time.
If you run netstat --tcp --numeric | grep 4444 you will see that the socket on the port 4444 is in TIME_WAIT state. Meaning that connection is closed, so it will be terminated in the timeout specified by the OS. After the timeout expires, the program executes without the error.

Command to check if the socket can bind to the port:
	$ netstat --tcp --numeric | grep 4444
