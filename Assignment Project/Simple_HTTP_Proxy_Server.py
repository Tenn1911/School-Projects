
import os
import sys
from socket import *
from urllib.parse import urlparse
from pathlib import Path

CACHE_DIR = "cache"
BUFFER_SIZE = 2048

def create_cache_dir():
    if not os.path.exists(CACHE_DIR):
        os.makedirs(CACHE_DIR)

def get_cache_filename(host,port,path):
    path = path.replace('/','_')
    return os.path.join(CACHE_DIR, (host+"_"+port+path))

def send_error(client_socket, status_code, reason, message):
    response = (
        f"HTTP/1.0 {status_code} {reason}\r\n"
        f"Content-Type: text/plain\r\n"
        f"Content-Length: {len(message)}\r\n"
        f"\r\n"
        f"{message}"
    )
    print(f"\n{response}\n")
    client_socket.sendall(response.encode())

def handle_request(client_socket):
    try:
        request = client_socket.recv(BUFFER_SIZE).decode()
        print(f"raw request:\n{request}")
        if not request:
            client_socket.close()
            return
        
        request_line = request.split('\r\n')[0].split()
        if len(request_line) < 2:
            send_error(client_socket,"400","Bad Request","Invalid request format")
            return
        
        method, url = request_line[0],request_line[1]

        if method != "GET":
            send_error(client_socket,"405", "Method Not Allowed", "405 Method Not Allowed")
            return
        
        parsed_url = urlparse(url)

        host = parsed_url.hostname
        if not host:

            send_error(client_socket,"400","Bad Request","Invalid host name")
            return
        
        port = parsed_url.port or 80
        path = parsed_url.path if parsed_url.path else '/'
        print(f"url: {url}\n")
        print(f"host: {host}, port: {port}, path: {path}\n")

        file_name = get_cache_filename(host,str(port),path)
        print(f"filename: {file_name}")
        #create dir just incase it doesn't exsist
        create_cache_dir()

        #check for file hit
        
        if os.path.exists(file_name):
            print(f"<<CACHE HIT>>\nServed from: {file_name}")
            with open(file_name, 'r') as file:
                while True:
                    chunk = file.read(BUFFER_SIZE)
                    if not chunk:
                        break
                    client_socket.sendall(chunk.encode())
            client_socket.close()
            return
        #handle file miss
        print("<<CACHE MISS>>")
        try:
                forward_socket = socket(AF_INET,SOCK_STREAM)
                print(f"Connecting to server\n")
                forward_socket.connect((host,port))
                msg = (
                     f"GET {path} HTTP/1.0\r\n"
                     f"Host: {host}\r\n"
                     "Connection: close\r\n"
                     "User-Agent: SimpleProxy/1.0\r\n"
                     "\r\n"
                )
                forward_socket.sendall(msg.encode())
                print(f"connection successful to {host},{port}\n")

                with open(file_name, 'w') as cache_file:
                    while True:
                        data = forward_socket.recv(BUFFER_SIZE)
                        if not data:
                            break
                        cache_file.write(data.decode())
                        print(f"Saved: type:{type(data)}, length:{len(data)} to the server\n")

                with open(file_name, 'r') as file:
                    while True:
                        chunk = file.read(BUFFER_SIZE)
                        if not chunk:
                            break
                        client_socket.sendall(chunk.encode())

                print("Sent data to client\n")
                client_socket.close()
                return


        except Exception:
                send_error(client_socket, "502", "Bad Gateway", "502 Bad Gateway")
        finally:
                forward_socket.close()
    except Exception as e:
        print(f"Error handling client: {e}")
    finally:
        client_socket.close()

def main():
    if len(sys.argv) != 2:
        print("Usage: python ProxyServer.py localhost")
        sys.exit(1)

    host = sys.argv[1]
    port = 8888

    create_cache_dir()

    server_socket = socket(AF_INET,SOCK_STREAM)
    server_socket.bind((host,port))
    server_socket.listen(1)
    print(f"Proxy running on {host}:{port}")

    while True:
        client_socket, address = server_socket.accept()
        print(f"\nConnection from: {address}\n")
        handle_request(client_socket)
        print("ready for next client")

if __name__ == "__main__":
    main()