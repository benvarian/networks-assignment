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
PORT = 1234  # Port to listen on (non-privileged ports are > 1023)
QBTYPE = "Python"
QTYPES = ['c', 'p']
class Nick_Socket:
    """ 
    send and receive packets.
    running loop, will listen for requests from TM 
    """

    def __init__(self, HOST, PORT):
        # inits. 
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # binds sock to a port
    # def bind(self):
        # self.sock.bind((self.HOST, self.PORT))

    # waits for TM to connect
    def connect_to_TM(self):
        # self.sock.listen()
        while(True):
            try:
                print("Trying to connect to TM...")
                self.sock.connect((HOST, PORT))  # connect to the server
                break
            except Exception as e:
                # timeout or some other error
                print(e)
            print("Fail: Could not connect to TM, Trying again in 2 seconds.\n")
            time.sleep(2)

        # connected to TM.
        # sending header.
        self.send_str("QB" + QBTYPE)
        
        # print("here.")
        # while(True):
        #     print(self.sock.recv(4096))
        #     time.sleep(0.5)
        #     print("here.")
        #     self.send_str("Hiya there.")
        #     time.sleep(1)
        # time.sleep(1)
        # self.send_str("QB" + QBTYPE)
        # self.send_str("QB" + QBTYPE)

    def send_str(self, msg):
        MSGLEN = len(msg)
        byte_msg = msg.encode()
        sent = self.sock.send(byte_msg)
        if (sent != MSGLEN):
            raise RuntimeError("couldnt send header.")
        
    # sends string along sock.
    def send_questions(self, msg):
        # as per current protocol 3 byte header -- one for type (char), 2 for length (2 byte int)
        MSGLEN = len(msg)
        # J for JSON
        byte_msg = b''.join(["J".encode(), MSGLEN.to_bytes(2, "little"), msg.encode()])
        print("\nsending questions:\nMSGLEN:",  MSGLEN, "\nmsg:", msg, "\n")
        totalsent = 0
        sent = self.sock.send(byte_msg)

    def send_mark(self, Mark):
        # as per current protocol 1 byte header -- one for type (char)
        # M for Mark
        byte_msg = b''.join(["M".encode(), Mark.to_bytes(2, "little")])
        print("\nsending marks:\nmsg:", Mark, "\n")
        totalsent = 0
        while totalsent < 3: # only three bytes to send ! double check though
            sent = self.sock.send(byte_msg)
            if sent == 0:
                raise RuntimeError("Socket Connection Broken")
            totalsent = totalsent + sent
            

    def wait_for_tm(self):
        # wait for req -- either mark ('m') or get qs ('q').
        # if m -- get qid, answer to send back mark.
        # if q -- get q_num, q_type.
        # chunks = []
        # bytes_recd = 0
        # if we need request id's of some sort, then we can include studentID or something of the type.

        # waits till receive anything.
        # msg = self.format(new_sock_receive())
        msg = False
        # loops until a msg is received
        while(not msg):
            try:
                print("waiting to receive a message...\n")
                msg = self.sock.recv(4096)
            except Exception as e:
                raise e
            if (msg == b''):
                raise Exception("broken pipline")
            elif(not msg):
                print("No req yet...")
                time.sleep(2)
            else:
                break
        print("Msg received!\nMsg:", msg, "\n")
        return msg

    def handle_req(self, msg):
        # chr converts bytes to unicode.
        mode_req = chr(msg[0])

        if (mode_req == 'm'):
            qid = chr(msg[1])
            ans = chr(msg[2:]) # answer can be a string

            # mark = question_bank.mark(qid, ans)
            # NOT IMPLEMENTED YET
            self.send_mark(qid, 1)
        elif (mode_req == 'q'):
            q_type = chr(msg[1])
            # if we choose str.
            q_num = int(msg[2:4].decode("utf-8"))
            # if we choose int.
            # q_num = int.from_bytes(msg[2:4], "little")

            print("qnum =", q_num)
            if (q_type not in QTYPES):
                print("invalid request. second val in q req should be in QTYPES.")
            questions = question_bank.get_JSON_qs(q_type, q_num)
            self.send_questions(questions)
        else:
            # something went wrong.
            # TODO: maybe maintain a count and close socket after three issues in a row.
            print("Request doesn't follow protocols, ignoring.")
            # raise Exception("Protocol broken. closing socket")

    def check_connection(self):
        try:
            # tries to read bytes without blocking without removing them from buffer (peek & don't wait)
            data = self.sock.recv(16, socket.MSG_DONTWAIT | socket.MSG_PEEK)
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
        # ends socket, ready to be restarted
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
        except Exception as e:
            print(e)
            
        try:
            self.sock.close()
        except Exception as e:
            print(e)

        self.sock = socket.socket()
    
    def main_loop(self):
        # receive, send... receive, send...
        self.connect_to_TM()
        print("\nBeginning main loop...\n")
        while True:
            if (self.check_connection()):
                try:
                    msg = self.wait_for_tm()
                    self.handle_req(msg)
                except Exception as e:
                    # if something is wrong, will be caught when checking connection
                    print(e)
                    self.restart_socket()
                    print("\nTrying to connect to TM...")
                    self.connect_to_TM()
                    # raise Exception("Something went wrong...")
            else:
                # connection has ended, restart_socket()
                print("Connection ended, Restarting...")
                self.restart_socket()
                print("\nTrying to connect to TM...")
                self.connect_to_TM()
                # self.wait_for_client_connect()
                # print("Connected to Client! Restarting main loop...\n")


# echo-server.py
new_sock = Nick_Socket(HOST, PORT)
# binds to HOST, PORT.
while(True):
    try:
        new_sock.main_loop()
    except Exception as e:
        print(e)
        new_sock.restart_socket()
        

