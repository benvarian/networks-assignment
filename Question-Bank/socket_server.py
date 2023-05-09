# send vs write, read vs recv, flush.
# send retruns how many bytse were sent through, this is our responsibility to handle. otherwise not all will go throguh.
# will want either a header containing how many bytes to send through, or a footer saying 'done'
# if recv returns 0, the connecting is closed on the otherside, or is in the process of closing.
# make sure to always end in sock.close()
# socket.setblocking(False) -- no blocking. research more.

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
    """ Class to handle sockets in python.
    So as to not have messy ridiculously long files.
    """

    def __init__(self, sock=None):
        # inits. creates a socket if not supplied
        if sock is None:
            self.sock = socket.socket(
                            socket.AF_INET, socket.SOCK_STREAM)
        else:
            self.sock = sock

    def bind(self, host, port):
        self.sock.bind((host, port))
        print("socket binded!")

    def wait_for_client(self):
        self.sock.listen()
        self.conn, self.addr = self.sock.accept()
        print("client connected!")

    def sendSTR(self, msg):
        # as per current made up protocol, send length of msg first and wait for ack.
        MSGLEN = len(msg)
        self.conn.send(MSGLEN.to_bytes(2, 'big'))
        # wait for ack
        self.receive()

        totalsent = 0
        while totalsent < MSGLEN:
            sent = self.conn.send(msg[totalsent:].encode())
            if sent == 0:
                raise RuntimeError("socket connection broken")
            totalsent = totalsent + sent

    

    def receive(self):
        chunks = []
        bytes_recd = 0
        # MSGLEN = 1024
        while bytes_recd == 0:
            chunk = self.conn.recv(2048)
            if chunk == b'':
                raise RuntimeError("socket connection broken")
            chunks.append(chunk)
            bytes_recd = bytes_recd + len(chunk)
        response = b''.join(chunks)
        if (response == "handshake failed"):
            raise RuntimeError("something went wrong...")
        
        return response.decode("utf-8") 
    

# wait for request...
# get request. parse it.
# solve request
# send len of ans (as header)
# send ans
# receive ack


 # echo-server.py
new_sock = Nick_Socket()
new_sock.bind(HOST, PORT)
new_sock.wait_for_client()

print("all cylinders firing, beginning question server.")

while(True):
    # waits for client request
    client_req = new_sock.receive()
    if not client_req:
        raise RuntimeError("Connection went wrong...")
    print("recieved: '{}'".format(client_req))

    reply = req_parse(client_req)
    new_sock.sendSTR(reply)
    break
    # what format should req be in? 
