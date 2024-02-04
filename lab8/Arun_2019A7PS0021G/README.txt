Steps to run the programs:
	1) gcc client.c -o client -lssl -lcrypto
	2) ./client [url]

You can use any link for testing, here is one : https://p.ip.fi/2WQo
It works with all the file formats

To check https : https://www.icicibank.com/
To check https text file : https://raw.githubusercontent.com/HappyKid117/cn-lab8-file/main/Riddle.txt
To check https pdf file : https://kpfu.ru/staff_files/F_1407356997/overview.pdf
To check http text file : http://p.ip.fi/2WQo

Since file name according to example link is 2WQo, the saved file name will also be 2WQo.

If content length is missing, then wait till the client timeouts for the file to be ready.
