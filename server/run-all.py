import time
import subprocess
import threading

from web.ipc.ipc import *
from web.web_main import *

class MyClass(threading.Thread):
    def __init__(self):
        self.stdout = None
        self.stderr = None
        threading.Thread.__init__(self)

    def run(self):
        p = subprocess.Popen('native/build/iot-server'.split(),
                             shell=False)

        self.stdout, self.stderr = p.communicate()


if __name__=='__main__':
    myclass = MyClass()
    myclass.start()
    time.sleep(0.2)
    ipc_inst = IPC()
    print("Python IPC connected successfully!!!")
    start(ipc_inst)
    myclass.join()