Author: Neeraj Suresh Narayanan
CCID: nnarayan
Student ID: 1666155

To start the server - 
./server [portNum]

To start the client - 
./client [portNum] [IPAddress] <inputfile.txt

Implementation Details - 
1. The server accepts multiple clients across multiple machines using the poll() function.
2. The server side output will be displayed in a "server.log" file, which will be available as soon as the server finishes execution.
3. Please note: The server.log file and the summary stats will only be displayed if the server shuts down 30 seconds after the last transaction from the client.
4. Server output will NOT be diplayed if the server is force shutdown using "ctrl-c". PLease wait 30 seconds for the server to shutdown to display the server side output.
4. Client side output will be displayed on the stdout.


Documentation for poll() Implementation - https://www.ibm.com/docs/en/i/7.2?topic=designs-using-poll-instead-select