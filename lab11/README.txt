
Ensure OpenSSL is installed on your computer

Steps to run the programs:
	1) gcc server.c -o server -lssl -lcrypto -pthread
	2) gcc client.c -o client -lssl -lcrypto -pthread
	3) ./server 4444
	4) ./client 4444 0.0.0.0
	5) ./client 4444 0.0.0.0
	6) Type "EXIT" in either of the clients to exit

Public Keys for server are currently hardcoded in server.c, I will implement taking them as filename/argument later.

Both clients can figure out which client they are (0 or 1) from the server's response.
According to their id given to them by server, they pick their private Keys.

The expression (id+1)%2 returns 1 for id=0 and vice versa. This way you can ask for "other" client's info knowing current client's ID.

The server creates a new <clientID>_<time> for every message as per the question. Remmber to delete these files regularly while testing.

The client.c file has a lot of weird code. Most of it is to implement pretty output. (The way chat keeps getting updated and the input textbox goes down).

Did not clean code, so there might be redundancies.

