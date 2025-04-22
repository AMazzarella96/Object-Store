# Sistemi Operativi Project a.y. 2018/19

## Introduction
The student is expected to implement an object store implemented as a client-server system, and intended to support requests to store and retrieve blocks of data from a large number of applications. The connection between clients and object store is through sockets on local domain. 
In particular, it will be necessary to implement:
- the server part (object store) as a stand-alone executable
- a library intended to be embedded in clients that interfaces with the object store using the protocol defined below
- an example client that uses the library to test the operation of the system

## Object-Store
The object store is an executable whose purpose is to receive requests from clients to store, retrieve, delete named blocks of data, called “objects.” The object store:
- manages a separate storage space for each client
- object names are guaranteed to be unique within a client's storage space
- client names are guaranteed to be all distinct

## Files & Structure

The project consists of two main files: mainServer.c, mainClient.c, and a utility directory
containing the files os_lib.c/h, related to the library for client-side functions, utils.c/h, in which is defined
a control macro and a set of support functions for client and server, and two files for the
management of operation reports: report.h (server-side), status.h (client-side).

- `mainServer.c`: The file represents the Server implementation of objectStore; defined within it is the
listening loop for connection requests from clients, which is responsible for creating
folders for each user, and the thread routine for serving requests. For the management of the
different clients, a bidirectional list of struct usr defined as a pair
<username, socket>, so as to uniquely identify the different requests from clients.
Two methods are provided for managing the list, such as insert and freelist. Insert provides
queues the newly created usr block, also updates a last pointer pointing
to the last element in the list, which is used as a parameter to pthread_create.
Freelist, on the other hand, takes care of deallocating the list when the tests are completed. The thread routine
is implemented by means of a switch case on an integer representing the request; for this
regard, it makes use of an opt enum and a strtoenum function to convert from a string to a
numeric value. Finally, a mechanism for signal handling has been defined to avoid
leaving the system in inconsistent situations; defined in a separate section.

- `mainClient.c`
The file represents the Client implementation of ObjectStore, in which at startup, after making
making sure that the parameters passed by argument are correct, a call is made
to the os_connect() function to establish the connection with the server, and then it
enter inside a switch case to distinguish the 3 operations: STORE, RETRIEVE and
DELETE. In the particular case of retrieve, a function was defined
filecompare(file, size) that has the task of verifying that the content of the received file is
the expected one, in which a file of size is generated on the spot and a
a string compare between the retrieve file and the latter. Having completed the required operation,
the os_disconnect() function is called on the output, which sends a request to
disconnect to the server.


- `os_lib.c`
File containing the implementation of the functions used by the client to send requests
to the server according to the defined communication protocol. Within it is defined a
global variable int sock related to the client socket, set by the os_connect() method in
connection phase; used by the other functions to send requests. For the exchange of
messages via socket between client and server were used, in addition to read and write, two functions
auxiliaries: readn and writen (in utils.c), to avoid partial reads or writes during sending or
reception of large block sizes (functions defined within utils.c). An additional
support function is checkresponse, which takes care of checking whether the message received
from the server is an “OK” or “KO” message.

- `utils.c/h`
(.h) Interface containing a macro for checking allocated pointers (CHECK_PTR),
an enum representing different operations, and a struct associating the enum value with a
string, which is used by the strtoenum function for passing from stiringa to enum.
(.c) File containing the implementation of utility functions used by client and server,
such as readn and writen to make up for partial read/write problems, split which has the
task of splitting requests word by word by returning an array of strings and the number
of split words, and finally the strtoenum mentioned above.

- `report.h`
Server-side interface in which a struct info has been defined for the purpose of reporting the status of the
system and related functions for updating parameters and printing the report. It consists of.
of the following parameters:
  - *users*: indicates the number of online users;
  - *objcount*: indicates the total number of files in the system;
  - *totsize*: indicates the total size of files present in the system.
Since these are functions called by the different threads, a mutex was declared for access in
mutual exclusion to the struct, except for addusr() and printrep() which are called only
by the main thread of the server.

- `status.h`
Client-side interface in which a struct stat has been defined for the purpose of reporting the status of the
operations performed by clients with associated functions for updating and printing the
statistics. It consists of the following parameters:
  - *op_count*: total count of executed operations;
  - *op_success*: number of successful operations;
  - *op_failed*: number of failed perations.
Being peculiar to each client, no mute exclusion mechanism was necessary.

## Signal Handling

Regarding signal management, a mechanism has been defined to prevent the server from
terminating leaving the system in an inconsistent state. The idea is that the server terminates only in the case
when there is no client connected. To do this, two functions, sighandler and
catcher, and two global variables: volatile sig_atomic_t sigflag initialized to 0, set within the
catcher function (volatile sig_atomic_t to ensure signal safe access) and int running to manage
task execution. The server's listening loop at each iteration tests sigflag to monitor the
reception of any signals and, in case of USR1, prints the report and resets the flag,
while if it intercepts a termination signal it proceeds to initiate termination phase in which it waits for
clients to finish and then deallocates all used resources and terminates the server.

**Operation**: Once installed, the signal manager sets up the facility for recording
signals. The signals registered are SIGINT, SIGTERM, SIGQUIT and SIGUSR1. The handler uses
a catcher function for signal recognition which, in the case of a termination signal,
resets the global running variable and in each case then sets the sigflag variable with the captured signal.

## General Workflow
Once the server is started, it will create the main socket for listening to requests
connection from the clients, after which it will start its listening cycle. Within it, for each
request, it will check that the header conforms to what is specified in the communication protocol
and then proceeds to create the folder and send the response message to the client.
Mirroring the server, the client as first makes a connection request through
a call of os_connect, after which it enters the switch case for requests commanded by the
second parameter passed as a command-line argument. Having done this within the server
an object of type usr is created, in which the request username and its socket are stored,
which is inserted within the user list using the insert function. After that, we
proceed by creating (or reallocating) the threadpool for the creation of a new thread.

At this point we switch to the th_task thread routine. The while loop has two conditions: one on
running (handled by the signal handler) and the other on the socket read, which is the one that handles the
reception of requests. Once we enter the while, the request is split by the split function and
the first word (the one representing the request in text format) is passed to the function
strtoenum, which returns the value of the enum corresponding to the request which will be used to
distinguish the different instances of the switch. The LEAVE is not present inside the switch but is located
at the output, since it is called independently following any request. Thus
like the store, retrieve and delete also affect all twenty corresponding files.

- `STORE`
As first thing in the client, the string representing the file name is constructed in the
test<length>.txt format, where <length> varies between 100 and 100002. After this is done we
we proceed with the creation of the block to be saved, structured as a sequence of 1 and 0 in the
even and odd positions respectively. Once the creation of the block is complete, the
called the os_store by passing it the name, block, and length. In the os_store we begin
by constructing the request header in STORE name len \n format and it is sent
to the server with a write. The server, once the header is split, converts len to integer with the
atoi function and constructs the path to the file by concatenating the username and filename.
A string of size len+1 is then allocated for receiving the file, and the server puts itself
on hold for readn. At this point the client will perform the writen of size len for sending
of the file and the server will receive it and save in the allocated string. Finally, the server performs an fwrite
to save the file and in case of problems a KO message is sent to the client, otherwise
the OK and it proceeds to deallocate the facilities used. On the client side, the response is checked
by the checkresponse.

- `RETRIEVE`
As with STORE, at the beginning the file name is generated and passed to the
os_retrieve which takes care of the creation of the request header. Once the 
the header the write is made to send to the server and we stand by for the response. The server, once it receives and splits the header, builds the path and does a fopen in read and, if it
returns NULL it means that the file does not exist and a KO message is sent, otherwise
it proceeds with the fread. Having reached this point, the response string is prepared,
always structured in two pieces, header and block, and the header is sent. The client
will check if the response is a KO message and if so will deallocate the structures
and return NULL, or if it is a positive response and proceeds to allocate the buffer for the
receive the file via readn. Finally, the server will perform the writen to send the file,
it will deallocate the resources. Once the file is received, the facilities of the
client and the os_retrieve returns the pointer to the file, which is checked with the function
filecompare.

- `DELETE`
Having constructed the string for the filename, the os_delete function is called, which, once the
constructed the header, sends the request to the server and goes into read for the response. Just as for
the previous cases, the path is constructed and a fopen in read is performed. In the event that
returned NULL, the KO message will be sent to the client, otherwise a fseek will be performed
on the file to calculate its length for reporting purposes (total store size) and you will
the remove function will be called. Again, the outcome of the remove is checked
and if there is an error a KO message is sent to the client, otherwise an OK.

- `LEAVE`
When the requested operation is finished, the function
os_disconnect, which performs a write for the request and read for the response, and will perform the
close of the socket. On the server side, once the OK is sent, the deallocation of all
resources and termination of the thread.
