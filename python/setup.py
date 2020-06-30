from setuptools import setup, find_packages

long_description = "Python interface to ASCII-Serial-Com, a human readable serial communication protocol."
requires = ["crcmod"]

setup(
    name="asciiserialcom",
    description="ASCII-Serial-Com",
    long_description=long_description,
    author="Justin Hugon",
    author_email="opensource AT hugonweb.com",
    version="1.0.beta",
    packages=find_packages(),
    license="GPLv3",
    classifiers=[
        "Development Status :: 4 - Beta",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
    ],
    provides=["asciiserialcom"],
    setup_requires=requires,
    install_requires=requires,
    entry_points={
        "console_scripts": [
            "asciiSerialComShell=asciiserialcom.asciiSerialComShell:main"
        ]
    },
)
