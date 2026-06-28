# Chat Server

A multi-client chat server and interactive terminal client written in C. Built from scratch with kqueue for event-driven I/O, a custom binary wire protocol, SQLite for persistence, and SHA-256 password hashing.

## Features

- **Room-based messaging** — join rooms, broadcast messages to everyone in the room
- **Direct messages** — send private messages to specific users
- **Message editing and deletion** — modify or soft-delete messages by ID
- **Message history** — load the last 50 messages when joining a room
- **Authentication** — register/login with SHA-256 hashed passwords
- **Persistence** — all users, rooms, and messages stored in SQLite

## Architecture

``` txt
┌──────────────┐         ┌──────────────────────────────────┐
│    Client    │◄──TCP──►│             Server               │
│  (kqueue:    │         │  kqueue event loop                │
│   stdin +    │         │  ┌────────────┐ ┌──────────────┐ │
│   socket)    │         │  │  Protocol   │ │   Handler    │ │
└──────────────┘         │  │  (framing)  │ │  (dispatch)  │ │
                         │  └────────────┘ └──────┬───────┘ │
                         │  ┌────────────┐ ┌──────┴───────┐ │
                         │  │    Auth     │ │   SQLite DB  │ │
                         │  │  (SHA-256)  │ │              │ │
                         │  └────────────┘ └──────────────┘ │
                         └──────────────────────────────────┘
```

**Wire protocol** — binary framed messages: 1-byte type, 4-byte length (network order), JSON payload.

```
┌──────────┬──────────┬────────────────┐
│ type (1) │ len (4)  │ payload (len)  │
└──────────┴──────────┴────────────────┘
```

## Dependencies

- **SQLite** — ships with macOS, install via package manager on Linux
- **CommonCrypto** — macOS only (for SHA-256 hashing)
- **[cJSON](https://github.com/DaveGamble/cJSON)** — included in `lib/`
- **[datastructures](https://github.com/hshei/datastructures)** — included in `lib/`, used for client tracking (hashmap)

## Build

```bash
make            # build the server
make client     # build the interactive client
```

## Usage

Start the server:

```bash
./chatserver
```

In separate terminals, start one or more clients:

```bash
./test-client
```

### Client Commands

| Command | Description |
|---|---|
| `/login <user> <pass>` | Register or login |
| `/join <room>` | Join a chat room |
| `/dm <user> <message>` | Send a direct message |
| `/edit <msg_id> <text>` | Edit a message |
| `/delete <msg_id>` | Delete a message |
| `/history` | Load room message history |
| `<text>` | Send a message to the current room |

### Example Session

```
/login alice secret123
/join general
hello everyone!
/dm bob hey, are you there?
/history
/edit 3 actually, I meant this
/delete 2
```

## Project Structure

``` bash
chatserver/
├── include/
│   ├── server.h        server state (kqueue, clients, db)
│   ├── client.h        per-connection client struct
│   ├── protocol.h      message types and frame functions
│   ├── handler.h       message dispatch and business logic
│   ├── db.h            SQLite operations
│   └── auth.h          password hashing and verification
├── src/
│   ├── main.c          entry point
│   ├── server.c        kqueue event loop, connection management
│   ├── protocol.c      frame parsing and building
│   ├── handler.c       login, send, edit, delete, DM, history
│   ├── db.c            user/room/message persistence
│   ├── auth.c          SHA-256 password hashing
│   └── client.c        client struct initialization
├── client/
│   └── client.c        interactive terminal client
├── lib/
│   ├── cJSON.c/h       JSON parsing
│   └── datastructures/ type-generic data structures library
├── Makefile
└── schema.sql          database schema
```

## Database Schema

See [`schema.sql`](schema.sql) for the full schema. Four tables:

- **users** — id, username, hashed password
- **rooms** — id, room name
- **messages** — id, user, room, text, soft-delete flag, timestamps
- **direct_messages** — id, sender, recipient, text

## Platform

Currently macOS only (kqueue, CommonCrypto). Linux support (epoll, OpenSSL) planned.
 