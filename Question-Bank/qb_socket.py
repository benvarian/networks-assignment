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
import sys
import random

HOST = "localhost"  # Standard loopback interface address (localhost)
PORT =  8080 # Port to listen on (non-privileged ports are > 1023)

# p for python
QB_SUBJECT = "P"
INIT_HEADER = "Q" + QB_SUBJECT
# C = in C, P = python. if not in list, will respond with something like "wrong qb, type not found"
QTYPES = ['C', 'P']
MARK_HEADER = "M"
QUESTION_HEADER = "Q"
ERROR_HEADER = "Error"
SIGN_OFF = "\r\n\r\n\0"

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
        self.send_str(INIT_HEADER)

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

    # sends a str, with a double carriage return.
    def send_str(self, msg):
        msg = msg + SIGN_OFF
        MSGLEN = len(msg)
        byte_msg = msg.encode()
        sent = self.sock.send(byte_msg)
        if (sent != MSGLEN):
            raise RuntimeError("Couldnt send string.")
        
    # sends string along sock
    def send_questions(self, msg):
        msg = QUESTION_HEADER + msg + SIGN_OFF
        MSGLEN = len(msg)
        byte_msg = msg.encode()

        print("\nsending questions:\nMSGLEN:",  MSGLEN, "\nmsg:", msg, "\n")

        sent = self.sock.send(byte_msg)
        if sent == 0:
            raise RuntimeError("Socket Connection Broken")

    def send_mark(self, Mark):
        # M for Mark
        msg = MARK_HEADER + str(Mark) + SIGN_OFF
        byte_msg = msg.encode()

        print("\nsending marks:\nmsg:", msg, "\n")
        sent = self.sock.send(byte_msg)
        if sent == 0:
            raise RuntimeError("Socket Connection Broken")
            

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
                # prevents infinite loop of broken pipelines.
                # temporary.
                raise Exception("Broken Pipeline")
            elif(not msg):
                print("No req yet...")
                time.sleep(2)
            else:
                break
        print("\nMsg received:", msg, "\n")
        return msg

    def handle_req(self, msg):
        # chr converts bytes to unicode.
        mode_req = chr(msg[0])

        if (mode_req == MARK_HEADER):
            qid = chr(msg[1])
            ans = msg[2:].decode("utf-8") # answer can be a string
            mark = random.randint(0,1)
            print("Marking:\n\tqid =", qid, "\n\tans =", ans, "\n")
            # mark = question_bank.mark(qid, ans)
            self.send_mark(qid, mark)
        elif (mode_req == QUESTION_HEADER):
            q_type = chr(msg[1])
            # read str
            q_num = int(msg[2:4].decode("utf-8"))
            # if we choose int instead of str.
            # q_num = int.from_bytes(msg[2:4], "little")

            print("qnum =", q_num)
            if (q_type not in QTYPES):
                print("Invalid Request. Second val of q req should be in QTYPES")
                self.send_str(ERROR_HEADER + ": Type not recognised.")
                return
            questions = question_bank.get_JSON_qs(q_type, q_num)
            self.send_questions(questions)
        else:
            # TODO: maintain a count and restart socket after three issues in a row or something of the like.
            print("Request doesn't follow protocol, ignoring.")
            self.send_str(ERROR_HEADER + ": Type not recognised.")

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
        # restarts connection.
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
        except Exception as e:
            print(e)
            
        try:
            self.sock.close()
        except Exception as e:
            print(e)
        # to prevent it spamming TM
        time.sleep(1)

        self.sock = socket.socket()
    
    def main_loop(self):
        # main loop, waiting for TM to send something.
        self.connect_to_TM()
        print("\nBeginning main loop...\n")
        while True:
            if (self.check_connection()):
                try:
                    msg = self.wait_for_tm()
                    self.handle_req(msg)
                except Exception as e:
                    print(e)
                    self.restart_socket()
                    self.connect_to_TM()
            else:
                # connection has ended, restart_socket()
                print("Connection ended, Restarting...")
                self.restart_socket()
                self.connect_to_TM()


# echo-server.py
try:
    PORT=int(sys.argv[1])
except:
    print("\nUsage:\n python3 qb_socket.py {port}")
    exit()

print("PORT =", PORT)
new_sock = Nick_Socket(HOST, PORT)
# binds to HOST, PORT.
while(True):
    try:
        new_sock.main_loop()
    except Exception as e:
        print(e)
        new_sock.restart_socket()
