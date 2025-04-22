# Sistemi Operativi Project developed during 3rd year of computer science bachelor's degree (2019)

The student is expected to implement an object store implemented as a client-server system, and intended to support requests to store and retrieve blocks of data from a large number of applications. The connection between clients and object store is through sockets on local domain. 
In particular, it will be necessary to implement:
- the server part (object store) as a stand-alone executable
- a library intended to be embedded in clients that interfaces with the object store using the protocol defined below
- an example client that uses the library to test the operation of the system

# Object-Store
The object store is an executable whose purpose is to receive requests from clients to store, retrieve, delete named blocks of data, called “objects.” The object store:
- manages a separate storage space for each client
- object names are guaranteed to be unique within a client's storage space
- client names are guaranteed to be all distinct

# Files & Structure

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



- os_lib.c
File containing the implementation of the functions used by the client to send requests
to the server according to the defined communication protocol. Within it is defined a
global variable int sock related to the client socket, set by the os_connect() method in
connection phase; used by the other functions to send requests. For the exchange of
messages via socket between client and server were used, in addition to read and write, two functions
auxiliaries: readn and writen (in utils.c), to avoid partial reads or writes during sending or
reception of large block sizes (functions defined within utils.c). An additional
support function is checkresponse, which takes care of checking whether the message received
from the server is an “OK” or “KO” message.

- utils.c/h
(.h) Interface containing a macro for checking allocated pointers (CHECK_PTR),
an enum representing different operations, and a struct associating the enum value with a
string, which is used by the strtoenum function for passing from stiringa to enum.
(.c) File containing the implementation of utility functions used by client and server,
such as readn and writen to make up for partial read/write problems, split which has the
task of splitting requests word by word by returning an array of strings and the number
of split words, and finally the strtoenum mentioned above.

- report.h
Server-side interface in which a struct info has been defined for the purpose of reporting the status of the
system and related functions for updating parameters and printing the report. It consists of.
of the following parameters:
- users: indicates the number of online users;
- objcount: indicates the total number of files in the system;
- totsize: indicates the total size of files present in the system.
Since these are functions called by the different threads, a mutex was declared for access in
mutual exclusion to the struct, except for addusr() and printrep() which are called only
by the main thread of the server.

- status.h
Client-side interface in which a struct stat has been defined for the purpose of reporting the status of the
operations performed by clients with associated functions for updating and printing the
statistics. It consists of the following parameters:
- op_count: total count of executed operations;
- op_success: number of successful operations;
- op_failed: number of failed perations.
Being peculiar to each client, no mute exclusion mechanism was necessary.
