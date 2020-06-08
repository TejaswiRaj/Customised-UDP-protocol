Programing Assignment 2:

Client using customized protocol on top of UDP protocol for requesting identification from server for access permission to the network.

Pre-requisite:
Please install gcc on Linux for C program compiler

Procedure to compile and run in Linux:

1. Copy the files 'client2.c', 'server2.c', 'sample_input_pa2.txt' and 'Verification_Database.txt' to the desired location.
2. Run the below commands to compile the C programs:
	gcc server2.c -o server2
	gcc client2.c -o client2
3. First the server should be started. To start the server, run:
	./server2
4. In a new terminal window, run below for running the client program:
	./client2
5. Packets would start transmitting and output would be visible