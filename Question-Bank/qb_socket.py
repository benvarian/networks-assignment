import socket
import qb_database
import time

# C = in C, P = python
QTYPES = ['C', 'P']

# Header variables for managing QB protocol
MARK_HEADER = "MARK\r\n"
QUESTION_HEADER = "QUESTIONS\r\n"
ERROR_HEADER = "ERROR\r\n"
ANSWER_HEADER = "ANSWER\r\n"
GETQUESTION_HEADER = "GETQUESTION\r\n"
END_HEADER = "\r\n"
END_MSG = "\r\n\0"

class QB_Socket_Connection:
    """Socket for handling TM connection.
        TM listens for QB connection request
        connection stays open, and TM requests from
        QB as needed.
    """

    def __init__(self, HOST, PORT, QB_TYPE):
        """init 

        Args:
            HOST (int): TM's host
            PORT (int): TM's port
        """
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.HOST = HOST
        self.PORT = PORT
        self.QB_SUBJECT = QB_TYPE
        self.QB_HEADER = "QB " + QB_TYPE + "\r\n"
        self.QB_DB = qb_database.QB_DB(self.QB_SUBJECT[0])

    def connect_to_TM(self):
        """Attempts to connect to the TM at given address.
            If it fails, tries again every 2 seconds.
        """
        while (True):
            try:
                print("Trying to connect to TM...")
                self.sock.connect((self.HOST, self.PORT))
                break
            except Exception as e:
                print(e)
            # connection failed
            print("Fail: Could not connect to TM, Trying again in 2 seconds.\n")
            time.sleep(2)
        # connected to TM.
        # sending header.
        self.send_init()

    def send_init(self):
        """Sends init HEADER to TM, to ensure everything is working correctly
            Format: 
            "QB {subject}
             
            \0"

        Raises:
            RuntimeError: failure to send a string, results in restarting of socket.
        """
        msg = self.QB_HEADER + END_HEADER + END_MSG
        MSGLEN = len(msg)
        byte_msg = msg.encode()
        sent = self.sock.send(byte_msg)
        if (sent != MSGLEN):
            raise RuntimeError("Couldnt send string.")

    def send_error(self, err):
        """Sends an error to TM, to let it know that something has gone wrong
            with it's request.
            Primarily used as a debugging tool.

            In form:
            "QB {subject}
             ERROR
             Error:typeOfError
             \0"

        Args:
            err (String): String describing the error.

        Raises:
            RuntimeError: Failure to send string, results in restarting of socket.
        """
        msg = self.QB_HEADER + ERROR_HEADER + "Error:" + err + END_MSG
        MSGLEN = len(msg)
        byte_msg = msg.encode()
        sent = self.sock.send(byte_msg)
        if (sent != MSGLEN):
            raise RuntimeError("Couldnt send string.")

    def send_questions(self, msg):
        """Sends Questions to the TM
            In form:
            "QB {subject}
             QUESTIONS
             
             qid:{qid}&type:{type}&question:{question}&{same for other questions...}
             \0"

        Args:
            msg (String): msg to be sent

        Raises:
            RuntimeError: if socket connection is broken
        """
        # msg = QB_HEADER + QUESTION_HEADER + END_HEADER + "QUESTIONS:" + msg + END_MSG
        msg = self.QB_HEADER + QUESTION_HEADER + END_HEADER  + msg + END_MSG
        MSGLEN = len(msg)
        byte_msg = msg.encode()

        print("\nsending questions:\nMSGLEN:",  MSGLEN, "\nmsg:", msg, "\n")

        sent = self.sock.send(byte_msg)
        if sent == 0:
            raise RuntimeError("Socket Connection Broken")

    def send_question(self, msg):
        """Sends a single question to the TM
            In form:
            "QB {subject}
             GETQUESTION
             
             {question string}
             \0"

        Args:
            msg (String): msg to be sent

        Raises:
            RuntimeError: if socket connection is broken
        """
        # msg = QB_HEADER + QUESTION_HEADER + END_HEADER + "QUESTIONS:" + msg + END_MSG
        msg = self.QB_HEADER + GETQUESTION_HEADER + END_HEADER  + msg + END_MSG
        MSGLEN = len(msg)
        byte_msg = msg.encode()
        print("\nsending questions:\nMSGLEN:",  MSGLEN, "\nmsg:", msg, "\n")

        sent = self.sock.send(byte_msg)
        if sent == 0:
            raise RuntimeError("Socket Connection Broken")
        
        
    # sends string along sock
    def send_answer(self, msg):
        """send_answer sends the answer to a given q from the TM
        sent in form:
        "QB {subject}
        ANSWER

        {answer as string}
        "

        Args:
            msg (String): answer to be sent

        Raises:
            RuntimeError: iff socket connection broken
        """
        msg = self.QB_HEADER + ANSWER_HEADER + END_HEADER + msg + END_MSG
        MSGLEN = len(msg)
        byte_msg = msg.encode()

        print("\nsending questions:\nMSGLEN:",  MSGLEN, "\nmsg:", msg, "\n")

        sent = self.sock.send(byte_msg)
        if sent == 0:
            raise RuntimeError("Socket Connection Broken")

    def send_mark(self, mark):
        """Sends mark of question to TM.
            of form:
            "QB {subject}
            MARK

            {mark (as string)}
            \0"

        Args:
            mark (int): mark to be sent.

        Raises:
            RuntimeError: Error if socket connection is broken.
        """
        msg = self.QB_HEADER + MARK_HEADER + END_HEADER + "MARK:" + str(mark) + END_MSG
        byte_msg = msg.encode()

        print("\nsending marks:\nmsg:", msg, "\n")
        sent = self.sock.send(byte_msg)
        if sent == 0:
            raise RuntimeError("Socket Connection Broken")
        
    def send_response(self):
        """Sends a response to TM's init p

        Raises:
            RuntimeError: if socket connection is broken
        """
        msg = "ACCEPTED PING"
        byte_msg = msg.encode()
        sent = self.sock.send(byte_msg)
        if sent == 0:
            raise RuntimeError("Socket Connection Broken")
        

    def wait_for_tm(self):
        """Waits on the TM to send a request, 
            either for marks or for questions.

        Raises:
            e: Broken socket.
            Exception: Broken socket.

        Returns:
            _type_: msg received.
        """
        msg = False
        # loops until a msg is received
        while (not msg):
            try:
                print("waiting to receive a message...\n")
                msg = self.sock.recv(4096)
            except Exception as e:
                raise e
            if (msg == b''):
                raise Exception("Broken Pipeline")
            elif (not msg):
                print("No req yet...")
                time.sleep(2)
            else:
                break
        print("\nMsg received:", msg, "\n")
        return msg

    def handle_req(self, msg):
        """Handles msg sent by TM
            Mark req in form:
                "MARK
                {qid}:{ans}
                "
            Question req in form:
                "QUESTIONS
                {subject}:{num}
                "
            req Answer in form:
                "ANSWER
                {qid}
                "
        Args:
            msg (String): Message sent by TM
        """
        msg = msg.decode("utf-8").split("\r\n")
        mode_req = msg[0] + "\r\n"
        print(f"MESSAGE RECEIVED: {msg}")
        if (mode_req == MARK_HEADER):
            qid, ans = msg[1].split(":")
            # mark = random.randint(0, 1)
            mark = self.QB_DB.mark(qid, ans)
            print("Marking:\n\tqid =", qid, "\n\tans =", ans, "\n")
            self.send_mark(qid, mark)
        elif (mode_req == QUESTION_HEADER):
            q_type, q_num = msg[1].split(":")

            print("qnum =", int(q_num))
            if (q_type not in QTYPES):
                print("Invalid Request. Second val of q req should be in QTYPES")
                self.send_error("q_typeError")
                return
            # mandatory extra long one-liner in order to be more 'pythonic'
            questions = ''.join(["&{}:{}".format(q[0], q[1]) for q in self.QB_DB.get_rand_qs(int(q_num))])
            questions.join("&")
            self.send_questions(questions)
        elif(mode_req == ANSWER_HEADER):
            # removes null terminator
            qid = int(msg[1][:-1])
            question = self.QB_DB.get_q_by_id(qid)
            answer = question[2]
            self.send_answer(answer)
        # handle pings from tm just by making an elif as its a viable header 
        elif(mode_req == GETQUESTION_HEADER):
            # removes null terminator
            qid = int(msg[1][:-1])
            question = self.QB_DB.get_q_by_id(qid)
            self.send_question(question[1])
        elif (mode_req == "TM\r\n"):
            print("TM PINGED QB, Responding with ACCEPTED PING")
            self.send_response()
            return
        else:
            # TODO: maintain a count and restart socket after three issues in a row or something of the like.
            print("Request doesn't follow protocol, sending Error.")
            self.send_error("q_modeError")

    def check_connection(self):
        """Checks connection with TM.

        Returns:
            Boolean: true if connection is working.
        """
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
            return False

    def restart_socket(self):
        """Restarts the socket
        """
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
        except Exception as e:
            print(e)

        try:
            self.sock.close()
        except Exception as e:
            print(e)
        # prevents it spamming TM if TM 
        # closes pipelines as they start.
        time.sleep(1)

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def main_loop(self):
        """The main operation loop for handling the socket.
        """
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
