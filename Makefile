RPCGEN=rpcgen

all: chebyshev_server chebyshev_client

chebyshev.h: chebyshev.x
	$(RPCGEN) -C chebyshev.x
	
chebyshev_svc.c: chebyshev.x
    $(RPCGEN) -C -m chebyshev.x > chebyshev_svc.c

chebyshev_xdr.c: chebyshev.x
    $(RPCGEN) -C -c chebyshev.x > chebyshev_xdr.c

chebyshev_server: chebyshev_server.c chebyshev_svc.c chebyshev_xdr.c chebyshev.h
    gcc -o chebyshev_server chebyshev_server.c chebyshev_svc.c chebyshev_xdr.c -lnsl

chebyshev_client: chebyshev_client.c chebyshev_xdr.c chebyshev.h
	gcc -o chebyshev_client chebyshev_client.c chebyshev_xdr.c -lnsl

clean:
    rm -f chebyshev_server chebyshev_client chebyshev.h chebyshev_svc.c chebyshev_xdr.c *.o