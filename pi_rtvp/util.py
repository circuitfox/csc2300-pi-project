from os.path import abspath, dirname, exists, join

def fullname(o):
    return o.__module__ + "." + o.__class__.__qualname__

def get_ffserver_conf():
    if exists("/etc/pi_rtvp/ffserver.rtvp.conf"):
        return "/etc/pi_rtvp/ffserver.rtvp.conf"
    else:
        return join(abspath(dirname(__file__)), "../conf/ffserver.rtvp.conf")
