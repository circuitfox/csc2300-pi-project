from os.path import abspath, dirname, exists
from os import killpg
import pi_rtvp.camera as camera
import pi_rtvp.kernel as kernel
import pi_rtvp.png as png
from pi_rtvp.util import get_ffserver_conf
from subprocess import Popen, PIPE, DEVNULL
from readchar import readchar

def input_loop(state):
    while True:
        prompt = input("> ").split(" ")
        if prompt[0] == "help":
            print("""USAGE:
set [yaw|pitch|output|kernel] [value] Set the given variable to a value
get [var]                             Get the value stored in a variable
get                                   Dump all program state
save [duration] (file)                Save a processed stream of <duration> seconds, to <file> if given or to the current output
stream                                Stream processed video over http
""")
        if prompt[0] == "exit":
            return
        if prompt[0] == "get":
            if len(prompt) == 1:
                get(state, None)
            else:
                get(state, prompt[1])
        if prompt[0] == "set":
            set(state, prompt[1], prompt[2])
        if prompt[0] == "save":
            if len(prompt) == 2:
                save_file(state, int(prompt[1]))
            elif len(prompt) == 3:
                save_file(state, int(prompt[1]), prompt[2])
        if prompt[0] == "stream":
            print("Press enter to end the stream")
            stream_file(state)

def get(state, var):
    if var == None:
        print(str(state))
    else:
        print(str(getattr(state, var)))

def set(state, var, value):
    if var in ["yaw", "pitch"]:
        try:
            int(value)
        except ValueError:
            return state
    elif var == "output":
        if not exists(dirname(abspath(value))):
             return state
        setattr(state, var, value)
        return state
    elif var == "kernel":
        try:
            setattr(state, var, kernel_from_name(value))
            return state
        except ValueError:
            return state
    else:
        print("var must be one of: yaw pitch output kernel")
        return state

def kernel_from_name(name):
    if name == "id":
        return kernel.id
    elif name == "mean":
        return kernel.mean
    elif name == "gaussian":
        return kernel.gaussian
    elif name == "sobel":
        return kernel.sobel
    elif name == "sobel-gaussian":
        return kernel.sobel_gaussian
    else:
        raise ValueError("Invalid Kernel")

ffmpeg_flags = ["ffmpeg", "-y", "-f", "image2pipe", "-vcodec", "png", "-r", "30",
                "-i", "-", "-vcodec", "libx264", "-r", "30"]

def save_file(state, duration, outfile=None, use_picam=True):
    def capture(cam, video, capture_fn):
        image = capture_fn(cam).to_greyscale()
        state.kernel.convolve_image(image).write(video.stdin)
    output = outfile or state.output
    video = Popen(ffmpeg_flags + [output], stdin=PIPE)
    frames = 0
    maxframes = duration * 30
    if use_picam:
        with camera.PiCamera(state.picam_path, 1280, 720, 30) as cam:
            while frames < maxframes:
                capture(cam, video, png.capture_picam)
                frames += 1
    else:
        with camera.USBCamera(state.usbcam_path, 1280, 720, 30) as cam:
            while frames < maxframes:
                capture(cam, video, png.capture_usbcam)
                frames += 1
    video.stdin.close()
    video.wait()
    return

def stream_file(state, use_picam=True):
    def stream(cam, stream, capture_fn):
        while readchar() != '\r':
                image = capture_fn(cam).to_greyscale()
                state.kernel.convolve_image(image).write(stream.stdin)
        stream_in.stdin.close()
        stream_in.wait()
        return
    conf = get_ffserver_conf()
    stream_server = Popen(["ffserver", "-f", conf, "&"], stdout=DEVNULL, stderr=DEVNULL)
    stream_in = Popen(ffmpeg_flags + ["http://localhost:8000/rtvp.ffm"], stdin=PIPE, stdout=DEVNULL, stderr=DEVNULL)
    if use_picam:
        with camera.PiCamera(state.picam_path, 1280, 720, 30) as cam:
            stream(cam, stream_in, png.capture_picam)
    else:
        with camera.USBCamera(state.usbcam_path, 1280, 720, 30) as cam:
            stream(cam, stream_in, png.capture_usbcam)
