from timeit import default_timer as timer
from dateutil import parser
import threading
import datetime
import socket
import time


class Client:

    def __init__(self, port):
        self.port = port
        self.slave_client = socket.socket()

    def connect(self):
        # connect to the clock server on local computer
        self.slave_client.connect(('127.0.0.1', self.port))

    def start_sending_time(self):
        while True:
            # provide server with clock time at the client
            self.slave_client.send(str(datetime.datetime.now()).encode())
            print("Recent time sent successfully", end="\n\n")
            time.sleep(5)

    def start_receiving_time(self):
        while True:
            # receive data from the server
            synchronized_time = parser.parse(self.slave_client.recv(1024).decode())
            print("Synchronized time at the client is: " + str(synchronized_time), end="\n\n")

    def initiate_slave_client(self):
        # start sending time to server
        print("Starting to receive time from server\n")
        send_time_thread = threading.Thread(target=self.start_sending_time)
        send_time_thread.start()

        # start receiving synchronized from server
        print("Starting to receiving " + "synchronized time from server\n")
        receive_time_thread = threading.Thread(target=self.start_receiving_time)
        receive_time_thread.start()


def run_client():
    client = Client(port=8080)
    client.connect()
    client.initiate_slave_client()



if __name__ == '__main__':
    
    # create and start 10 threads, each running a separate client
    threads = []
    for i in range(10):
        thread = threading.Thread(target=run_client)
        thread.start()
        threads.append(thread)

    # wait for all threads to finish
    for thread in threads:
        thread.join()
