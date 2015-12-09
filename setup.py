from setuptools import setup, find_packages, Extension
from setuptools.command.test import test as TestCommand
from numpy.distutils.misc_util import get_numpy_include_dirs
from codecs import open
from os import path

here = path.abspath(path.dirname(__file__))

class Tox(TestCommand):
    user_options = [("tox-args=", "a",
                     "Arguments to pass to tox")]
    def initialize_options(self):
        TestCommand.initialize_options(self)
        self.tox_args = None

    def run_tests(self):
        import tox
        import shlex
        args = self.tox_args
        if args:
            args = shlex.split(self.tox_args)
        errno = tox.cmdline(args=args)
        sys.exit(errno)

with open(path.join(here, "README.rst"), encoding="utf-8") as f:
    long_description = f.read()

setup(
    name="pi_rtvp",
    version="0.0.1",
    description="Real-Time Video Processing with the Raspberry Pi",
    long_description=long_description,
    url="https://github.com/circuitfox/csc2300-pi-project",
    author="Chris Stankus",
    author_email="cdstankus@gmail.com",
    license="MIT",
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Environment :: Console",
        "License :: OSI Approved :: MIT License",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: Python :: 3",
    ],
    packages=find_packages(exclude=['docs', 'tests']),
    install_requires=["RPi.GPIO", "picamera", "readchar", "numpy"],
    tests_require=["tox"],
    cmdclass={"test": Tox},
    data_files=[("/etc/pi_rtvp", [path.join(here, "conf/ffserver.rtvp.conf")])],
    ext_modules=[
        Extension("pi_rtvp.cutil",
                  ["pi_rtvp/py_cutil.c", "pi_rtvp/cutil.c"],
                  extra_compile_args=["-std=c99"]),
        Extension("pi_rtvp.matrix", 
                  ["pi_rtvp/py_matrix.c", "pi_rtvp/matrix.c"],
                  extra_compile_args=["-std=c99"]),
    ],
    include_dirs=get_numpy_include_dirs(),
    entry_points={
        "console_scripts": [
            "pi_rtvp=pi_rtvp:main"
        ],
    },
)

