# Simple Local Client-Server Messenger
Simple local client-server messenger that supports concurrent communication between multiple connected clients, along with broadcasting messages and sending files.

Made using Windows C API.

## Description

The **server** uses _Windows Named Pipe_ objects for communication with connected clients and also uses a _thread-pool_ for managing requests and tasks. It permits user registration and communication only for logged in users.
User credentials will be stored in **C:\registration.txt**.

The **client** has user input for commands. The client will try to connect to the **server** and will shut down if the connection fails, either because the **server** connection count reached max, or the **server** has not been yet started.

#### Available client commands
```
1. echo [string]
    - Prints [string] on screen

2. register [username] [password]
    - Registers a new user
    - Username restrictions: a-zA-Z0-9 only
    - Password restrictions: at least a capital (A-Z) and a non alphanumeric character; cannot contain spaces or commas; at least 5 characters long;

3. login [username] [password]
    - Logs the user in

4. logout
    - Logs out the user; no action if no user is logged in

5. msg [user] [message]
    - Sends the message content to the specified user, if this one is 'online' (logged in)
    - The sender must be logged in

6. broadcast [message]
    - Sends a message to every logged in user
    - The sender must be logged in

7. sendfile [user] [file path]
    - Sends a file to a logged in user, if the sender is logged in, as well;
    - The file is sent in the background, so the user can execute other commands

8. list
    - Shows all logged in users

9. exit
    - Exists the client application, logging out the user.
```

## Usage

### Server

```bash
Compile "server/" and "shared/" as "server.exe"

./server.exe [max connection count permitted]
```

### Client

```bash
Compile "client/" and "shared/" as "client.exe"

./client.exe 
```

