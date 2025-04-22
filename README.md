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
