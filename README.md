# multiple-client-server-chat-program

In this project I have implemented a client-server chat application using TCP sockets.
There is a single server and multiple clients communicating with the server. The server
process can handle at most 5 concurrent connections. Each client process will open a new
connection with the server and to handle each client request, the server will create a new
child process.

Follow the pdf file for detailed infos.

Supported Commands:
1. /active : To display all the available active clients that are connected to the server.

2. /send <dest client id> <Message> : To send message to the client corresponding to
its unique id.
  
3. /broadcast <Message> ​ : Message should be broadcasted to all the active clients.
  
4. /makegroup <client id1> <client id2> ... <client idn> : A group with unique id will be
made including all the mentioned clients along with the admin client.
  
5. /sendgroup <group id> <Message>​ : The sender should be in the group to transfer
the message to all his peers of that group. The message should be send to all the
peers along with group info.
  
6. /activegroups : To display all the groups that are currently active on server and the
sender is a part of.

7. /makegroupreq <client id1> <client id2> ... <client idn> : A group having unique id
should be made with currently only the admin client. The request message for
joining the group should be notified to all the specified clients. Clients can respond
to join that group.
  
8. /joingroup <group id> ​ : If this message is sent by a client having the request for
joining the group, then he will be added to group immediately.
  
9. /quit : The client will be removed from the server. This client will be removed from
all the active groups.10. /activeallgroups​ : To display all the groups which are active on the server.

10. /activeallgroups​ : To display all the groups which are active on the server.

11. /joingroup <group id> : If this message is sent by a client having the request for
joining the group, then he will be added to group immediately. Otherwise a request
should be passed to the admin of that group and if admin responds to the request
positively then he should be joined to that group.
  
12. /sendfile  <client id> <file_name> : Send a file to a client having the particular client id.
