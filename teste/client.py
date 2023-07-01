from socket import create_connection

socket = create_connection(("vanaheim", 8080))

socket.send(b"Hello, world!")

print(socket.recv(1024))

socket.close()