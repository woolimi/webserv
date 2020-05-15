## What is socket ?
Sockets allow communication between __two different processes on the same or different machines__. To be more precise, it's a way to talk to other computers using standard Unix file descriptors. In Unix, every I/O action is done by writing or reading a file descriptor. A file descriptor is just an integer associated with an open file and it can be a network connection, a text file, a terminal, or something else.

To a programmer, a socket looks and behaves much like a low-level file descriptor. This is because commands such as read() and write() work with sockets in the same way they do with files and pipes.

## socket communication Server <-> Client
<table>
    <thead>
        <tr>
            <th>Server</th>
            <th>Client</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td colspan=2>socket() : make socket. 
                It has own number, has no ip, port number yet.</td>
        </tr>
        <tr>
            <td>bind() : bind IP/port to socket </td>
            <td rowspan=3> connect() : client try to connect 
                to server with server IP/Port
            </td>
        </tr>
        <tr>
            <td>listen() : server wait client </td>
        </tr>
        <tr>
            <td>accept() : server accept client connection </td>
        </tr>
        <tr>
            <td colspan=2>
               read()/write() : server and client exchange data each other.
            </td>
        </tr>
        <tr>
            <td colspan=2>close() : close socket</td>
        </tr>
    </tbody>
</table>

* How the server works
```
socket() -> bind() -> listen() 
-> accept()
-> read()/write()
-> close()
```

## resource
* [what do you need to know to build a simple http-server](https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa)
