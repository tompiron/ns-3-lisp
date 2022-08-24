# NS-3 LISP with Publish/Subscribe

This repository contains a copy of the ns-3.27 source code, more information can be found in the [NS3_README file](NS3_README).
This is a fork of E. Marechal's work that can be found [here](https://github.com/Emeline-1/ns-3_LISP_NAT).

We added capabilities to the simulator in order to run simulations with the Publish/Subscribe extension of LISP.
A link toward the thesis linked to this work will be available soon.

## Installation steps (Linux)

In order to install the project, first, clone the repository.

```
$ git clone https://github.com/tompiron/ns-3-lisp.git
$ cd ns-3-lisp
```

Then make sure the requirement to run LISP are installed.

```
$ sudo apt install -y python2 gcc g++
```

Make also sure that python2 is the default python installation.
You can do it however you want, using [conda](https://docs.conda.io/en/latest/) or simply setting the default system wide, your choice.
My personal choice is to create the link `ln -s /usr/bin/python2 /bin/python`.

Then we need to configure and compile ns-3. This takes a while.
```
$ ./waf configure
$ ./waf
```
## Running the scripts

There are three scripts in our code: final, simplePubSub and simplelisp.
The first one is the one we used in our work. The second is a simple example of Publish/Subcribe. And the last is a simple example of LISP.

To run a script simply run one of the following:
```
$ ./waf --run [script_name]
$ ./waf --run final --command-template "%s [# runs] [seed] [# senders] [# receivers]"
```
For the first script, a directory named ´output´ in the root of the repo need to be created and the results will be stored there.
The two other scripts simply output pcap files in the root directory of the repo.
