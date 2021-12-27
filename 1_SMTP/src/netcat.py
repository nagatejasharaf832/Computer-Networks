import socket

class UsageError(Exception): pass

class Netcat(object):
    def connect(self, host, port):
        if not isinstance(host, str):
            raise UsageError("Host must be ip address or resolvable url")
        try:
            addr = socket.getaddrinfo(host, port, proto=socket.IPPROTO_TCP)[0][4]
        except IndexError:
            raise UsageError("Unknown hostname or IP")
        
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect(addr)
        
        

    def read(self, length=1024):
        return self.socket.recv(length)

    def write(self, data):
        self.socket.send(data.encode('utf-8'))
    def close(self):
        self.socket.close()

sender_email = input("enter email id: ")
reciever_email = input("enter receiver email: ")
subject = input("enter subject of email: ")
f = open("file.txt","w+")
for i in range(10):
    f.write("This is line %d\r\n" % (i+1))
f.close()
f=open("file.txt", "r")
body_data = f.read()
f.close()
nc = Netcat()        
nc.connect('mail-relay.iu.edu',25)      
print(nc.read(1024).decode("utf-8"))
nc.write("helo silo.sice.indiana.edu\r\n")
print(nc.read(1024).decode("utf-8"))
nc.write("MAIL FROM:{}\r\n".format(sender_email))
print(nc.read(1024).decode("utf-8"))
nc.write("RCPT TO:{}\r\n".format(reciever_email))
print(nc.read(1024).decode("utf-8"))
nc.write("DATA\r\n")
nc.write("SUBJECT:{}\r\n".format(subject))
print(nc.read(1024).decode("utf-8"))
nc.write("\r\n {}".format(body_data))
nc.write("\r\n.\r\n")
print(nc.read(1024).decode("utf-8"))
