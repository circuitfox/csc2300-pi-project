from setuptools import setup, find_packages
from setuptools.command.test import test as TestCommand
from codecs import open
from os import path

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

here = path.abspath(path.dirname(__file__))

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
    install_requires=["RPi.GPIO", "picamera"],
    tests_require=["tox"],
    cmdclass={"test": Tox},
    entry_points={
        "console_scripts": [
            "pi_rtvp=pi_rtvp:main"
        ],
    },
)

