# upp-7.0.0-travis
[![Build Status](https://travis-ci.com/sunjay/upp-7.0.0-travis.svg?token=i5M6iNHVbWshsp6jHWxw&branch=master)](https://travis-ci.com/sunjay/upp-7.0.0-travis)

A version of the u++-7.0.0 installation compiled on a Travis CI machine.

An example `.travis.yml` file that should allow you to use this installation:

```yml
sudo: required
dist: trusty
language: cpp

install:
  - sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
  - sudo apt-get update -y
  - sudo apt-get install -y g++-8
  - g++-8 --version
  - g++ --version # old version
  - sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 --slave /usr/bin/g++ g++ /usr/bin/g++-8
  - g++ --version # should be g++-8
  - wget https://github.com/sunjay/upp-7.0.0-travis/archive/master.zip -O u++-7.0.0.zip
  - unzip u++-7.0.0.zip 'upp-7.0.0-travis-master/*'
  - mv upp-7.0.0-travis-master u++-7.0.0
  - export PATH=$PATH:$(pwd)/u++-7.0.0/bin
  - export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/u++-7.0.0/lib
  - which u++
  # u++ --version has an exit code of 1 for some reason
  - u++ --version || true

script:
  #TODO: Replace this with your "make" commands or whatever
  - echo "custom build commands go here"
```
