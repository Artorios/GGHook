import socket

HOST = "127.0.0.1"
PORT = 9090
i = 0
while True:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        conn, addr = s.accept()
        with conn:
            print(f"Connected by {addr}")
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                print(data)
                conn.sendall(data)
        print("[%d] %s\n" % (i, "-"*48))
        i = i + 1