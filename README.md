# messagequeues 
## System V IPC Queues X11 Motif Example

### Introduction

This repository contains a simplified example on message Queues.
It uses System V IPC as message queues and Motif as X11 Toolkit.
It is not intended as a ready to use library but as a basic example on how to use SysV queues.

### Compilation

Requieres Motif and X11 development libraries. File /etc/xbus is used for `ftok`

```
# touch /etc/xbus
$ make xbus
$ make xclient
```

### Usage

Clients allow registering against the bus and sending messages.
Bus reads client register requests and distributes to all registered clients the messages.
Below image shows one bus and four clients launched. Notice how the fourth client did not get registered and hence it is not receiving the messages.

![Screenshot](/messagequeues_screenshot.png?raw=true "Screenshot")

### Caveats

This repository is just a basic example it lacks certain functionality:
- Deregistration.
- Timeouts.
- Termination of queues.

Additionally Andrew from #FreeBSD at Freenode correctly pointed out some issues:
- There is no need to open queues at the bus side per client, and that `mtype` value could be used to identify destination client. Hence all clients and bus would share the same queue.
- ftok takes only the 8 least significant bits from `proj_id`, in the current implementation it would not be that difficult to get collisions on the lowest byte of the PID, as PIDs are used as `proj_id`.
- There is at least one bug in the FreeBSD kernel which deals `key_t` as `int` instead of as `long`. Apparently that is affecting `ipcs` but not code written in C.

