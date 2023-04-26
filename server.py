import matplotlib.pyplot as plt
from functools import reduce
from dateutil import parser
import threading
import datetime
import random
import socket
import time

class ClockServer:
    def __init__(self, port=8080):
        self.port = port
        self.client_data = {}
        self.selected_clients = []

        self.master_server = socket.socket()
        self.master_server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    def start(self):
        self.master_server.bind(('', self.port))
        self.master_server.listen(10)
        print("Clock server started...\n")

        # start making connections
        print("Starting to make connections...\n")
        master_thread = threading.Thread(target=self.startConnecting)
        master_thread.start()

        # start synchronization
        print("Starting synchronization parallelly...\n")
        sync_thread = threading.Thread(target=self.synchronizeAllClocks)
        sync_thread.start()

    def startConnecting(self):
        while True:
            # accepting a client / slave clock client
            master_slave_connector, addr = self.master_server.accept()
            slave_address = str(addr[0]) + ":" + str(addr[1])

            print(slave_address + " got connected successfully")

            current_thread = threading.Thread(
                            target=self.startReceivingClockTime,
                            args=(master_slave_connector, slave_address))
            current_thread.start()

    def startReceivingClockTime(self, connector, address):
        while True:
            # receive clock time
            clock_time_string = connector.recv(1024).decode()
            clock_time = parser.parse(clock_time_string)
            clock_time_diff = datetime.datetime.now() - clock_time

            self.client_data[address] = {
                "clock_time": clock_time,
                "time_difference": clock_time_diff,
                "connector": connector
            }

            if len(self.client_data) == 3 and len(self.selected_clients) < 3:
                self.selected_clients.append(address)

            print("Client Data updated with: "+ str(address), end="\n\n")
            time.sleep(0.1)

    def plotSelectedClientTime(self):
        if len(self.selected_clients) == 3:
            fig, ax = plt.subplots(figsize=(10, 5))
            ax.set_title("Local Time of Selected Clients")
            ax.set_xlabel("Time")
            ax.set_ylabel("Client Address")

            while True:
                x = []
                y = []
                for client in self.selected_clients:
                    x.append(datetime.datetime.now())
                    y.append(client)

                ax.plot(x, y, 'o')
                plt.pause(0.1)

    def getAverageClockDiff(self):
        current_client_data = self.client_data.copy()
        time_difference_list = list(client['time_difference'] for client_addr, client in current_client_data.items())
        sum_of_clock_difference = sum(time_difference_list, datetime.timedelta(0, 0))
        average_clock_difference = sum_of_clock_difference / len(current_client_data)

        return average_clock_difference

    def synchronizeAllClocks(self):
        while True:
            print("New synchronization cycle started.")
            print("Number of clients to be synchronized: " + str(len(self.client_data)))

            if len(self.client_data) > 0:
                average_clock_difference = self.getAverageClockDiff()
                for client_addr, client in self.client_data.items():
                    try:
                        synchronized_time = datetime.datetime.now() + average_clock_difference
                        client['connector'].send(str(synchronized_time).encode())
                    except Exception as e:
                        print("Something went wrong while " + "sending synchronized time " + "through " + str(client_addr))
            else:
                print("No client data. Synchronization not applicable.")
            
            print("\n\n")
            time.sleep(5)

if __name__ == '__main__':
    # Trigger the Clock Server
    clock_server = ClockServer(port=8080)
    clock_server.start()
