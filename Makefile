all:
	g++ mainClient.cpp tands.c -o client
	g++ mainServer.cpp tands.c -o server
	groff -man clientman.1 -T pdf > clientman.pdf
	groff -man serverman.1 -T pdf > serverman.pdf