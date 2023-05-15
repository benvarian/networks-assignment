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
# Port to listen on (non-privileged ports are > 1023)
SUBJECTS = ["PYTHON", "C"]
try:
    PORT=int(sys.argv[1])
    QB_TYPE = str(sys.argv[2])
    if (QB_TYPE not in subjects): raise Exception("Fail init args")
except:
    print("\nUsage:\n python3 qb_socket.py {port} {qb_type}")
    exit()



# p for python
QB_SUBJECT = QB_TYPE
QB_HEADER = "QB " + QB_SUBJECT + "\r\n"
# C = in C, P = python. if not in list, will respond with something like "wrong qb, type not found"
QTYPES = ['C', 'P']
MARK_HEADER = "MARK\r\n"
QUESTION_HEADER = "QUESTIONS\r\n"
ERROR_HEADER = "ERROR\r\n"

END_HEADER = "\r\n"

END_MSG = "\r\n\0"

# init the QB_DB
QB_DB = question_bank.QB_DB(QB_SUBJECT[0])

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
        while (True):
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
        self.send_init()

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
    def send_init(self):
        msg = QB_HEADER + END_HEADER + END_MSG
        MSGLEN = len(msg)
        byte_msg = msg.encode()
        sent = self.sock.send(byte_msg)
        if (sent != MSGLEN):
            raise RuntimeError("Couldnt send string.")

    def send_error(self, err):
        msg = QB_HEADER + ERROR_HEADER + "Error:" + err + END_MSG
        MSGLEN = len(msg)
        byte_msg = msg.encode()
        sent = self.sock.send(byte_msg)
        if (sent != MSGLEN):
            raise RuntimeError("Couldnt send string.")

        
    # sends string along sock
    def send_questions(self, msg):
        # msg = QB_HEADER + QUESTION_HEADER + END_HEADER + "QUESTIONS:" + msg + END_MSG
        msg = QB_HEADER + QUESTION_HEADER + END_HEADER  + msg + END_MSG
        MSGLEN = len(msg)
        byte_msg = msg.encode()

        print("\nsending questions:\nMSGLEN:",  MSGLEN, "\nmsg:", msg, "\n")

        sent = self.sock.send(byte_msg)
        if sent == 0:
            raise RuntimeError("Socket Connection Broken")

    def send_mark(self, mark):
        # M for mark
        msg = QB_HEADER + MARK_HEADER + END_HEADER + "MARK:" + str(mark) + END_MSG
        byte_msg = msg.encode()

        print("\nsending marks:\nmsg:", msg, "\n")
        sent = self.sock.send(byte_msg)
        if sent == 0:
            raise RuntimeError("Socket Connection Broken")
        
    def send_response(self):
        msg = "ACCEPTED PING"
        byte_msg = msg.encode()
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
        while (not msg):
            try:
                print("waiting to receive a message...\n")
                msg = self.sock.recv(4096)
            except Exception as e:
                raise e
            if (msg == b''):
                # prevents infinite loop of broken pipelines.
                # temporary.
                raise Exception("Broken Pipeline")
            elif (not msg):
                print("No req yet...")
                time.sleep(2)
            else:
                break
        print("\nMsg received:", msg, "\n")
        return msg

    def handle_req(self, msg):
        # chr converts bytes to unicode.
        msg = msg.decode("utf-8").split("\r\n")
        mode_req = msg[0] + "\r\n"

        if (mode_req == MARK_HEADER):
            qid, ans = msg[1].split(":")
            # mark = random.randint(0, 1)
            mark = QB_DB.mark(qid, ans)
            print("Marking:\n\tqid =", qid, "\n\tans =", ans, "\n")
            self.send_mark(qid, mark)
        elif (mode_req == QUESTION_HEADER):
            q_type, q_num = msg[1].split(":")

            print("qnum =", int(q_num))
            if (q_type not in QTYPES):
                print("Invalid Request. Second val of q req should be in QTYPES")
                self.send_error("q_typeError")
                return
            questions = ''.join(["qid:{}&type:{}&question:{}&".format(q[0], q[1], q[2]) for q in QB_DB.get_rand_qs(int(q_num))])
            self.send_questions(questions)
        # handle pings from tm just by making an elif as its a viable header 
        elif (mode_req == "TM\r\n"):
            print("TM PINGED QB Responding with ACCEPTED PING")
            self.send_response()
            return
        else:
            # TODO: maintain a count and restart socket after three issues in a row or something of the like.
            print("Request doesn't follow protocol, sending Error.")
            self.send_error("q_modeError")

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
                    print("here theres an error")
                    self.restart_socket()
                    self.connect_to_TM()
            else:
                # connection has ended, restart_socket()
                print("Connection ended, Restarting...")
                self.restart_socket()
                self.connect_to_TM()

# echo-server.py
def main():

    print("PORT =", PORT)
    TM_socket = Nick_Socket(HOST, PORT)
    # binds to HOST, PORT.
    while(True):
        try:
            TM_socket.main_loop()
        except Exception as e:
            print(e)
            TM_socket.restart_socket()

def test():
    q_num = 5
    questions = QB_DB.get_rand_qs(int(q_num))
    questions = ''.join(["qid:{}:type:{}:question:{}".format(q[0], q[1], q[2]) for q in questions])
    print(questions)
    msg = QB_HEADER + QUESTION_HEADER + END_HEADER + "QUESTIONS:" + questions + END_MSG
    print("resulting msg is: \n\n:::\n" + msg)

# test()
main()
