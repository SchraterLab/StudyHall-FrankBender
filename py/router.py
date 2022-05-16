from ctypes import *
sofile = "./build/lib/prizm.so"
Prizm = CDLL(sofile)

print("Starting shared object c")
# class Encrypt:
#     def hash():

Prizm.wonder()
# class Session:
#     id = Encrypt.hash()
#     def __init__(self, authenticate):


# class RoutingEngine:
#     'Common base class for all employees'
#     empCount = 0

#     def __init__(self, session):
#         self.session = Session.authenticate(session)
#         RoutingEngine.empCount += 1

#     def parseUrl(self, url):
#         print("Total Employee %d", RoutingEngine.empCount)

#     def printUrl(self, url):
#         print("Url: %s", url)