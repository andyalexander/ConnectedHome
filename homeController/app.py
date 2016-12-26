__author__ = 'Andrew'
from flask import Flask
import ctypes
import os
import subprocess

app = Flask(__name__)
isWindows = True if os.name == 'nt' else False

PLEX_LOCATION = r"C:\Program Files (x86)\Plex Home Theater\Plex Home Theatre.exe"

@app.route('/tv/ejectcd')
def ejectcd():
    if isWindows:
        ctypes.windll.WINMM.mciSendStringW(u"open D: type cdaudio alias d_drive", None, 0, None)
        ctypes.windll.WINMM.mciSendStringW(u"set d_drive door open", None, 0, None)
        return 'ejected'
    else:
        return 'not windows OS, eject cd called'

@app.route('/tv/runapp')
def runapp():
    if isWindows:
        subprocess.call([PLEX_LOCATION])
        print 'Running...' + PLEX_LOCATION
        return 'ok'
    else:
        return 'non-wintel, tried to run Plex'

if __name__ == '__main__':
    print "Running in debug mode"
    app.run(debug=True)