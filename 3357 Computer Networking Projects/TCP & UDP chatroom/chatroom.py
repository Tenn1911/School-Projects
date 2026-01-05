from socket import *
from threading import Thread
from threading import *
import select
import sys

class ServerTCP:
    def __init__(self, server_port):
        self.server_port = server_port
        self.server_addr = 'localhost'
        
        self.clients = {}
        self.run_event = Event()
        self.handle_event = Event()
        
        self.server_socket = socket(AF_INET, SOCK_STREAM)
        self.server_socket.bind((self.server_addr,self.server_port))
        self.server_socket.listen(1)

        

    def accept_client(self):
        try:

            #Use select to check if server socket is ready to accept (1 second timeout)
            ready, _, _ = select.select([self.server_socket], [], [], 1.0)

            if not ready:
                return False  # No connection available
            
            client_socket, client_address = self.server_socket.accept()
            message_bytes = client_socket.recv(1024)
            if not message_bytes:
                print("No message recieved")  # client closed connection
                return
            # decode for logging/processing
            message = message_bytes.decode().strip()

            name_list = self.clients.values()
            if message in name_list:
                response = 'Name already taken'
                client_socket.sendall(response.encode())
                return False

            client_socket.sendall('Welcome'.encode())    

            self.clients[client_socket] = message
            print(self.clients)

            self.broadcast(client_socket,'join')

            # start a new thread per client
            thread = Thread(target=self.handle_client, args=(client_socket,), daemon=True)
            thread.start()

            print(f"started thread for: {self.clients[client_socket]}")
            return True
        except KeyboardInterrupt:
            raise KeyboardInterrupt
        except Exception as e:
            print(f"Error accept client: {e}")
            return False
        
    def close_client(self, client_socket):
        try:
            if client_socket not in self.clients:
                print("User not in dict.")
                return False
            
            self.clients.pop(client_socket)
            client_socket.close()
            print(f"new clients: {self.clients}")
            return True
        
        except Exception as e:
            print(f"Error in close client: {e}")
            return False
        
    def broadcast(self, client_socket_sent, message):
        try:
            if message.lower() == 'join':
                broadcast_message = f'User {self.clients[client_socket_sent]} joined'
            elif message.lower() == 'exit':
                broadcast_message = f'User {self.clients[client_socket_sent]} left'
            else:
                broadcast_message = f'{self.clients[client_socket_sent]}: {message}'
            #print(f"sending broadcast {broadcast_message}")
            for client_socket in self.clients:
                #print("AAAAAAAAAAA")
                if client_socket == client_socket_sent:
                    #print("AAAAAAAAAAA")
                    continue
                else:
                    #print(f"sending to{socket}")
                    client_socket.sendall(broadcast_message.encode())
            #print("AAAAAAAAAAA")
            
        except Exception as e:
            print(f"Error in broadcast when send the message: {message}: {e}")

    def shutdown(self):
        msg = 'server-shutdown'
        try:
            for client_socket in self.clients:
                client_socket.sendall(msg.encode())
                client_socket.close()
            self.clients.clear()
            self.run_event.set()
            self.handle_event.set()
            self.server_socket.close()
        except Exception as e:
            print(f"Error in shutdown: {e}")

    def get_clients_number(self):
        return len(self.clients)
    
    def handle_client(self, client_socket):
            try:
                while not self.handle_event.is_set():
                    print(f"waiting for {self.clients[client_socket]} to send a message")
                    
                    message = client_socket.recv(1024).decode()
                    if not message:
                        break

                    self.broadcast(client_socket,message) 

                    if message.lower() == 'exit':
                        self.close_client(client_socket)
                        break             
            except Exception as e:
                print(f"Error in handle client: {e}")
                
    def run(self):
        try:
            print(f"TCP CHATROOM\nServer Side\nReceive connections on port{self.server_port}\nCtrl+C to shut down server\nwaiting for connections\n")
            while not self.run_event.is_set():
                self.accept_client()
        except KeyboardInterrupt:
            self.shutdown()
        except Exception as e:
            print(f"Error in run: {e}")
        finally:
            self.shutdown()

class ClientTCP:
    def __init__(self, client_name, server_port):
        self.server_addr = 'localhost'
        self.client_socket = socket(AF_INET, SOCK_STREAM)
        self.server_port = server_port
        self.client_name = client_name
        self.exit_run = Event()
        self.exit_receive = Event()

    def connect_server(self):
        try:
            self.client_socket.connect((self.server_addr, self.server_port))
            self.client_socket.sendall(self.client_name.encode())
            #print("name sent")
            msg = self.client_socket.recv(1024).decode()
            #print("name recv")
            print(msg)
            if msg == "Welcome":
                return True
            return False
        except Exception as e:
            print(f"Error: {e}")
        
    def send(self, text):
        try:
            self.client_socket.sendall(text.encode())
        except Exception as e:
            print(f"Error: {e}")

    def receive(self):
        try:
            while not self.exit_receive.is_set():
                #print("2")
                ready, _, _ = select.select([self.client_socket], [], [], 1.0)
                if not ready:
                    continue 
                msg = self.client_socket.recv(1024).decode()
                print("3")
                
                if not msg:  # Empty message means server disconnected
                    print("\nServer disconnected")
                    self.exit_run.set()
                    self.exit_receive.set()
                    break

                if msg == 'server-shutdown':
                    self.exit_run.set()
                    self.exit_receive.set()
                
                self.delete_last_line()
                print(f'{msg}')
                sys.stdout.write(f"{self.client_name}: ")
                sys.stdout.flush()
        except KeyboardInterrupt:
            raise KeyboardInterrupt 
        except Exception as e:
            print(f"Error: {e}")


    def run(self):
        try:
            if not self.connect_server():
                print("Failed to connect to server")
                return
            print("1")
            receive_thread = Thread(target=self.receive,daemon=True)
            receive_thread.start()
            print("4")
            while not self.exit_run.is_set():
                try:
                    sys.stdout.write(f"{self.client_name}: ")
                    sys.stdout.flush()
                    # Get user input
                    message = input()
                    
                    # Check if user wants to exit
                    if message.lower() == 'exit':
                        self.send('exit')
                        self.exit_run.set()
                        self.exit_receive.set()
                        continue
                    
                    # Send the message to the server
                    self.send(message)
                
                except KeyboardInterrupt:
                    raise KeyboardInterrupt
        except KeyboardInterrupt:
            try:
                self.send('exit')
            except:
                pass
            self.exit_receive.set()  
        except Exception as e:
            print(f"Error in run method: {e}")

    def delete_last_line(self):
        "Use this function to delete the last line in the STDOUT"

        #cursor up one line
        sys.stdout.write('\x1b[1A')

        #delete last line
        sys.stdout.write('\x1b[2K')
 

class ServerUDP:
    def __init__(self, server_port):
        self.server_port = server_port
        self.server_socket = socket(AF_INET,SOCK_DGRAM)
        self.server_socket.bind(('localhost',self.server_port))
        self.broadcast_index = 0

        self.clients = {}
        self.messages = []
        self.is_shutdown = False
        
    def accept_client(self, client_addr, message):
        try:
            if message in self.clients.values():
                self.server_socket.sendto('Name already taken'.encode(),client_addr)
                return False
            
            self.server_socket.sendto('Welcome'.encode(),client_addr)
            self.clients[client_addr] = message
            self.messages.append((client_addr,f'User {message} joined'))
            self.broadcast()
            return True
        except Exception as e:
            print(f"Error in run method: {e}")
    
    def close_client(self, client_addr):
        try:
            if client_addr not in self.clients:
                print("User addr not in client dict")
                return False
            name = self.clients[client_addr]
            self.clients.pop(client_addr)
            self.messages.append((client_addr,f'User {name} left'))
            self.broadcast()
            return True
        except Exception as e:
            print(f"Error in run method: {e}")
    
    def broadcast(self):
        try:
            while self.broadcast_index < len(self.messages):
                client_addr, message_text = self.messages[self.broadcast_index]
                
                for client in self.clients:
                    if client == client_addr:
                        continue
                    self.server_socket.sendto(message_text.encode(), client)
                
                print(f'"{message_text}" has been broadcasted')
                self.broadcast_index += 1
                
        except Exception as e:
            print(f"Error in broadcast method: {e}")
        
    def shutdown(self):
        if self.is_shutdown:
            return
        message = 'server-shutdown'
        self.messages.append(("None",message))
        self.broadcast()
        self.clients.clear()
        self.server_socket.close()
        self.is_shutdown = True

    def get_clients_number(self):
        return len(self.clients)
    
    def run(self):
        print(f"UDP CHATROOM\nServer Side\nReceive connections on port {self.server_port}\nCtrl+C to shut down server\nwaiting for connections\n")
        try:
            while True:
                #print("wating to reciving new message")
                ready, _, _ = select.select([self.server_socket], [], [], 1.0)
                if not ready:
                    continue  # No connection available
                
                rcv_data, client_address = self.server_socket.recvfrom(2048)
                message = rcv_data.decode()
                name,message_text = message.split(':',1)

                print(f'recveied:{message}')

                if message_text == 'join':
                    print(f'{name} has joined')
                    self.accept_client(client_address,name)
                    print(f'list of currnet users:{self.clients}')
                    continue

                elif message_text == 'exit':
                    print(f'{name} has exited')
                    self.close_client(client_address)
                    continue

                else:
                    print("registered user, appending and broadcasting message")
                    self.messages.append((client_address,message))
                    self.broadcast()

        except KeyboardInterrupt:
            self.shutdown()
        except Exception as e:
            print(f"Error in run method: {e}")
        finally:
            self.shutdown()

class ClientUDP:
    def __init__(self, client_name, server_port):
        self.server_addr = 'localhost'
        self.client_socket = socket(AF_INET, SOCK_DGRAM)
        self.server_port = server_port
        self.client_name = client_name
        self.exit_run = Event()
        self.exit_receive = Event()


    def connect_server(self):
        try:
            #print("sending join")
            self.send('join')
            #print("sent join")
            rcv_msg = self.client_socket.recvfrom(2028)
            msg = rcv_msg[0].decode().strip()
            #print(f"recv {msg}")
            if msg == 'Welcome':
                #print("accept")
                return True
            return False
        except Exception as e:
            print(f"Error in connect_server method: {e}")
        
    def send(self, text):
        try:
            msg = f'{self.client_name}:{text}'
            self.client_socket.sendto(msg.encode(),(self.server_addr,self.server_port))
        except Exception as e:
            print(f"Error in send method: {e}")

    def receive(self):
        try:
            while not self.exit_receive.is_set():
                #print("2")
                ready, _, _ = select.select([self.client_socket], [], [], 1.0)
                if not ready:
                    continue 

                msg = self.client_socket.recvfrom(1024)
                msg = msg[0].decode()

                print("3")
                
                if not msg:  # Empty message means server disconnected
                    print("\nServer disconnected")
                    self.exit_run.set()
                    self.exit_receive.set()
                    break

                if msg == 'server-shutdown':
                    self.exit_run.set()
                    self.exit_receive.set()
                    self.delete_last_line()
                    print(f'{msg}')
                    continue
                         
                self.delete_last_line()
                print(f'{msg}')
                sys.stdout.write(f"{self.client_name}: ")
                sys.stdout.flush()
        except KeyboardInterrupt:
            try:
                self.send('exit')
            except:
                pass
            self.exit_receive.set()    
        except Exception as e:
            print(f"Error in recieve method: {e}")

    def run(self):
        try:
            if not self.connect_server():
                print("Failed to connect to server")
                return
            print("1")
            receive_thread = Thread(target=self.receive,daemon=True)
            receive_thread.start()
            print("4")

            while not self.exit_run.is_set():
                try:
                    sys.stdout.write(f"{self.client_name}: ")
                    sys.stdout.flush()
                    # Get user input
                    message = input()
                    
                    # Check if user wants to exit
                    if message.lower() == 'exit':
                        self.send('exit')
                        self.exit_run.set()
                        break
                    
                    # Send the message to the server
                    self.send(message)
                
                except KeyboardInterrupt:
                    try:
                        self.send('exit')
                    except:
                        pass
                    self.exit_receive.set()  
                    break
        except KeyboardInterrupt:
            try:
                self.send('exit')
            except:
                pass
            self.exit_receive.set()  
        except Exception as e:
            print(f"Error in run method: {e}")

    def delete_last_line(self):
        "Use this function to delete the last line in the STDOUT"

        #cursor up one line
        sys.stdout.write('\x1b[1A')

        #delete last line
        sys.stdout.write('\x1b[2K')

def main():
    pass   

if __name__ == "__main__":
    main()

