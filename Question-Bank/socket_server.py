# send vs write, read vs recv, flush.
# send retruns how many bytse were sent through, this is our responsibility to handle. otherwise not all will go throguh.
# will want either a header containing how many bytes to send through, or a footer saying 'done'
# if recv returns 0, the connecting is closed on the otherside, or is in the process of closing.
# make sure to always end in sock.close()
# socket.setblocking(False) -- no blocking. research more.
# send seems better than sendall.

import socket
import json
import question_bank
import time

HOST = "localhost"  # Standard loopback interface address (localhost)
PORT = 65433  # Port to listen on (non-privileged ports are > 1023)

def req_parse(request):
    # needs to handle type qs, num qs.
    # first should be type, rest should be num?.
    # for now just always send ten.
    questions = question_bank.gen_qs(request)
    JSONQs = json.dumps(questions)
    print(JSONQs)
    return JSONQs


class Nick_Socket:
    """ 
    send and receive packets.
    running loop, will listen for requests from TM 
    """

    def __init__(self, HOST, PORT):
        # inits. 
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.HOST = HOST
        self.PORT = PORT

    # binds sock to a port
    def bind(self):
        self.sock.bind((self.HOST, self.PORT))

    # waits for TM to connect
    def wait_for_client_connect(self):
        self.sock.listen()
        self.conn, self.addr = self.sock.accept()

    # sends string along sock.
    def send_questions(self, msg):
        # as per current protocol 3 byte header -- one for type (char), 2 for length (2 byte int)
        MSGLEN = len(msg)   
        byte_msg = b''.join(["J".encode(), MSGLEN.to_bytes(2, "little"), msg.encode()])
        print("\nsending:\n MSGLEN:",  MSGLEN, "\nmsg:", msg, "\n")
        totalsent = 0
        while totalsent < MSGLEN:
            sent = self.conn.send(byte_msg)
            if sent == 0:
                raise RuntimeError("socket connection broken")
            totalsent = totalsent + sent

    # currently not needed
    def receive_long(self):
        chunks = [ ]
        bytes_recd = 0
        chunk = self.conn.recv(4096)
        MSGLEN = chunk[0]

        while bytes_recd < MSGLEN:
            chunk = self.conn.recv(4096)
            if chunk == b'':
                raise RuntimeError("socket connection broken")
            chunks.append(chunk)
            bytes_recd = bytes_recd + len(chunk)
        response = b''.join(chunks)
        return response.decode("utf-8") 

    def receive(self):
        msg = self.conn.recv(4096)
        print("Msg received!\n Msg: ", msg, "\n")
        if (not msg):
            raise Exception('No message received.')
        # msg will have a few headers, could (?) parse them here to make that work simpler.
        return msg

    def listen(self):
        # wait for req -- either mark ('m') or get qs ('q').
        # if m -- get qid, answer to send back mark.
        # if q -- get q_num, q_type.
        # chunks = []
        # bytes_recd = 0
        # if we need request id's of some sort, then we can include studentID or something of the type.

        # waits till receive anything.
        # client_req = self.format(new_sock_receive())
        client_req = new_sock.receive()
        print(client_req)

        # chr converts bytes to unicode.
        mode_req = chr(client_req[0])

        if (mode_req == 'm'):
            qid = chr(client_req[1])
            ans = chr(client_req[2])

            mark = question_bank.mark(qid, ans)
            # NOT IMPLEMENTED YET
            # self.send_mark(qid, mark)

        elif (mode_req == 'q'):
            q_num = chr(client_req[1])
            q_type = int.from_bytes(client_req[2:4], "big")

            questions = question_bank.get_JSON_qs(q_num, q_type)
            self.send_questions(questions)
        else:
            # something went wrong.
            print("something went wrong with request, skipping.")
            # raise Exception("Protocol broken. closing socket")


    def check_connection(self):
        try:
            # tries to read bytes without blocking without removing them from buffer (peek & don't wait)
            data = self.conn.recv(16, socket.MSG_DONTWAIT | socket.MSG_PEEK)
            print(data)
            print(len(data))
            return len(data) != 0
        except BlockingIOError:

            return True  # socket is open and reading from it would block
        except ConnectionResetError:
            return False  # socket was closed for some other reason
        except Exception as e:
            print("Unexpected Exception when checking if socket is connected:")
            print(e)
            print("\n\n")
            return True
        
    def restart_socket(self):
        # ends socket, and restarts it.
        # very likely we'll want more error checking for PORTs being already used.
        self.conn.shutdown(socket.SHUT_RDWR)
        self.conn.close()
        # self.sock.shutdown
        # self.bind()
    
    def main_loop(self):
        # receive, send... receive, send... 
        # if no requests for some amount of time, we
        # can close in the future to 
        # set up a new socket later.

        self.bind()
        print("Socket binded to: ", HOST, " : ", PORT)

        print("Waiting for Client...")
        self.wait_for_client_connect()
        
        print("Connected to Client! Beginning main loop...\n")
        
        while True:
            if (self.check_connection()):
                try:
                    print("Waiting for request...")
                    self.listen()
                except Exception as e:
                    # if something is wrong, will be caught when checking connection
                    print(e)
                    self.restart_socket()
                    print("\nWaiting for new Client...")
                    self.wait_for_client_connect()
                    # raise Exception("Something went wrong...")
            else:
                # connection has ended, restart_socket()
                print("Connection ended, Restarting...")
                self.restart_socket()
                print("\nWaiting for new Client...")
                self.wait_for_client_connect()
                # self.wait_for_client_connect()
                # print("Connected to Client! Restarting main loop...\n")
                # raise Exception("Something went wrong...")
                
# echo-server.py
new_sock = Nick_Socket(HOST, PORT)
# binds to HOST, PORT.
try:
    new_sock.main_loop()
except Exception as e:
    print(e)
    # new_sock.restart_socket()
    new_sock.sock.close()

