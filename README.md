# messagequeues

### Introduction

This repository contains a simplified example on message Queues.
It uses System V IPC as message queues and Motif as X11 Toolkit.
It is not intended as a ready to use library but as a basic example on how to use SysV queues.

### Compilation

Requieres Motif and X11 development libraries.

```
# touch /etc/xchart
make xbus
make xclient
```

### Usage

Clients allow registering against the bus and sending messages.
Bus reads client register requests and distributes to all registered clients the messages.


