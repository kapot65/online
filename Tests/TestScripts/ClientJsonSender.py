import socket
import struct
import json
import time

def createMachineHeader(jsonMeta, data = ''):
    binary_header = '#!'
    millis = int(round(time.time() * 1000))
    #current time
    binary_header += struct.pack('Q', millis)[4:]
    #binary header type
    binary_header += struct.pack('I', 0)
    #meta type
    binary_header += struct.pack('I', 0)
    #meta length
    binary_header += struct.pack('I', len(jsonMeta) + 4)
    #data type
    binary_header += struct.pack('I', 0)
    #data length
    binary_header += struct.pack('I', len(data))
    binary_header += '#!\r\n'
    return binary_header

def readMachineHeader(data):
    header = dict()
    header['type'] = struct.unpack('I', data[2:6])
    header['time'] = struct.unpack('I', data[6:10])
    header['metaType'] = struct.unpack('I', data[10:14])
    header['metaLength'] = struct.unpack('I', data[14:18])
    header['dataType'] = struct.unpack('I', data[18:22])
    header['dataLength'] = struct.unpack('I', data[22:26])
    return header
    

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#s.connect(('192.168.111.131', 33669))
s.connect(('192.168.111.125', 39812))

message = {
            "type":"command",
            "block":"divider1",
            "command_type":"check_init",
          }

meta = json.dumps(message)
binaryHeader = createMachineHeader(meta)

s.send(binaryHeader + meta + '\r\n\r\n')

answer = s.recv(1024)
header = readMachineHeader(answer)
answerMeta = json.loads(answer[30:(30 + header['metaLength'][0])])
print answerMeta
s.close()