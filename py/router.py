from ctypes import *
sofile = "./build/lib/prizm.so"
Prizm = CDLL(sofile)

# class Encrypt:
#     def hash(toAuth = ""):
#         print("Hashing")

# class Session:
#     id = Encrypt.hash()
#     def __init__(self, toAuth):
#         print("Starting session")
#         Encrypt.hash()


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


# Server.main.argtypes = c_int,POINTER(c_char_p)
# args = (c_char_p * 3)(b'abc',b'8081',b'./frontend')
# Server.main(len(args),args)
Prizm.wonder()
Prizm.main()