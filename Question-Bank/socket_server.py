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

HOST = "localhost"  # Standard loopback interface address (localhost)
PORT = 65435  # Port to listen on (non-privileged ports are > 1023)

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
    def bind(self, host, port):
        self.sock.bind((host, port))
        print("socket binded!")

    # waits for TM to connect
    def wait_for_client_connect(self):
        self.sock.listen()
        self.conn, self.addr = self.sock.accept()
        print("client connected!")

    # sends string along sock.
    def send_questions(self, msg):
        # as per current protocol 3 byte header -- one for type (char), 2 for length (2 byte int)
        MSGLEN = len(msg)   
        sent = self.conn.send(MSGLEN)
        byte_msg = b''.join(["J".encode(), MSGLEN.encode(2, "big"), msg.encode()])
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
        if (not msg):
            raise Exception('Connection failed')
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

        mode_req = client_req[0]
        
        if (mode_req == 'm'):
            qid = client_req[1]
            ans = client_req[2]

            mark = question_bank.mark(qid, ans)
            self.send_mark(qid, mark)
        elif (mode_req == 'q'):
            q_num = client_req[1]
            q_type = client_req[2]

            questions = question_bank.get_JSON_qs(q_num, q_type)

            self.send_questions(questions)
        else:
            # something went wrong.
            print("something went wrong with request, skipping.")
            raise Exception("Protocol broken. closing socket")


    def check_connection(self):
        try:
            # tries to read bytes without blocking without removing them from buffer (peek & don't wait)
            data = self.sock.recv(16, socket.MSG_DONTWAIT | socket.MSG_PEEK)
            return len(data) == 0
        except :
            # can be check against specific errors if needed.
            return False
        
    def restart_socket(self):
        # ends socket, and restarts it.
        # very likely we'll want more error checking for PORTs being already used.
        self.sock.shutdown()
        self.sock.close()
        self.sock.bind(self.HOST, self.PORT)
    
    def main_loop(self):
        # receive, send... receive, send... 
        # if no requests for some amount of time, we
        # can close in the future to 
        # set up a new socket later.

        self.bind()
        self.wait_for_client_connect()
        while True:
            if (self.check_connection()):
                try:
                    self.listen()
                except:
                    # if something is wrong, will be caught when checking connection.
                    print("something went wrong")
                    break
            else:
                # connection has ended, restart_socket()
                self.restart_socket()
                self.wait_for_client_connect()
                
# echo-server.py
new_sock = Nick_Socket(HOST, PORT)
# binds to HOST, PORT.
new_sock.main_loop()
