CSE30264: Programming Assignment 2
* Chris Foley (cfoley)
* Catherine Markley (cmarkley)
* Bailey Blum (bblum1)

Files

pg2client/Makefile

  Make file for the client

pg2client/myftp.c

    This is our client for our PA02 TCP assignment. The client begins by establishing a connection
    to the already-running server. It then accepts operations from the user (DNLD: Download,
    UPLD: Upload, LIST: List, MKDR: Make Directory, RMDR: Remove Directory, CHDR: Change Directory,
    CRFL: Create File, RMFL: Remove File, QUIT: Quit) and handles them appropriately via handler
    methods. The client loops to allow the user to enter multiple operations. It loops until the
    user enters QUIT.

pg2server/Makefile

  Make file for the server

pg2server/myftpd.c

    Server for our PA02 TCP assignemnt. The server opens a port and goes into "wait for connection" state.
    Server accepts a connection from a client. The server then waits for commands from the client (DNLD: Download,
    UPLD: Upload, LIST: List, MKDR: Make Directory, RMDR: Remove Directory, CHDR: Change Directory,
    CRFL: Create File, RMFL: Remove File, QUIT: Quit). It then handles each operation appropriately by calling
    a handler for each opeartion. The server loops as to allow multiple operations to be called. It does not allow multiple clients.


Commands to run the code on student01:
  Within pg2server/ :
      make
      ./myftp student01.cse.nd.edu 41002

  Within pg2client/ :
      make
      ./myftpd 41002


The port numbers can be replaced with other port numbers, but must match.
The hostname passed into the client must match the machine the server is running on.
