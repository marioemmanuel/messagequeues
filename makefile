xbus:           bus.o
                cc -std=c89 bus.o -o xbus -L/usr/local/lib -I/usr/local/include -lc -lXm -lXt -lX11 -pthread

bus.o:          bus.c
                cc -std=c89 -c bus.c -o bus.o -I/usr/local/include

xclient:        client.o
                cc -std=c89 client.o -o xclient -L/usr/local/lib -I/usr/local/include -lc -lXm -lXt -lX11 -pthread

client.o:       client.c
                cc -std=c89 -c client.c -o client.o -I/usr/local/include
