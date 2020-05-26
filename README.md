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

## curl
1. -X
choose method, POST, GET, HEAD, PUT, DELETE
2. -H
redifine header request value
3. --data
4. --resolve
Provide a custom address for a specific host and port pair. 
```
-- resolve <host:port:addr[,addr]...>
```

## telnet
```
telnet> open localhost 8080
Connecting To 127.0.0.1...
GET /index.html HTTP/1.0
(Hit enter twice to send the terminating blank line ...)
... HTTP response message ...
```

## nginx
```
nginx -c ~/mynginx.conf
```

## How to work together with github
### 1. Clone data from github
```
git clone [remote repository URL]
```
### 2. Create new branch for adding new code
```
git branch [branch_name]
git checkout [branch_name]
// if you want to do at once
git checkout -b [branch_name]
```
### 3. Commit and push to github
```
git add -A
git commit -m [commit_message]
git push -u origin [branch_name]
```
### 4. Ask pull request on github
### 5. Project manager check which code is changed and merge it on github
### 6. Syncronize between remote and local
```
git checkout master
git pull origin master
```

## resource
* [what do you need to know to build a simple http-server](https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa)
* [rfc7230](https://tools.ietf.org/html/rfc7230)
* [rfc7231](https://tools.ietf.org/html/rfc7231)
* [rfc7232](https://tools.ietf.org/html/rfc7232)
* [rfc7233](https://tools.ietf.org/html/rfc7233)
* [rfc7234](https://tools.ietf.org/html/rfc7234)
* [rfc7235](https://tools.ietf.org/html/rfc7235)
* [HTTP basics](https://www.ntu.edu.sg/home/ehchua/programming/webprogramming/HTTP_Basics.html)
