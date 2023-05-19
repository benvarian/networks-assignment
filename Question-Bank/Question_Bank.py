import sys
from qb_socket import *

SUBJECTS = ["PYTHON", "C"]
try:
    # Init input params
    HOST = sys.argv[1]
    PORT = int(sys.argv[2])
    QB_TYPE = str(sys.argv[3])
    if (QB_TYPE not in SUBJECTS): raise Exception("Fail init args")
except:
    print("\nUsage:\n python3 qb_socket.py {TM-Host} {port} {qb_type}")
    exit()

print("TM ADDRESS = ", str(HOST) + ":" + str(PORT))

def main():
    TM_socket = QB_Socket_Connection(HOST, PORT, QB_TYPE)
    while(True):
        try:
            TM_socket.main_loop()
        except Exception as e:
            print("Something went wrong... Restarting socket.")
            TM_socket.restart_socket()

main()
