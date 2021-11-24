# Example for OpenCL Benchmark: Matrix multiplication

## Linux

Check that you have the latest NVIDIA or AMD kernel drivers first! For OpenCL:

```bash
make
./mult
```

This was tested on Ubuntu/Pop!OS 20.04 and 21.04

## example output 
### (January 2021 - AMD Ryzen 5 3600 (1 hyperthreaded core / 12) vs. AMD RX580

```
device 0: Ellesmere
Status: SUCCESS
Results:
        A[0] = 2
        B[0] = 4
        C[0] = 32768
Mean execution time:
        1 CPU: 6450.41 ms;                        NOTE- REALLY "HYPERTHREADED CORE" 
        GPU: 51.112 ms.
Performance gain: 125.201x
```


# Comments

Maybe this is pointless but it's a very good explanation of why multiple compilers and barely-open-standard stuff makes
it less accessible to developers.  We had fun playing with this OpenCL with very simple coarse-grained models with
polyelectrolytes in the presence of counterions (think GS Manning 1970's for a good baseline theory that even experts
would forget or never find) to simulate atomistic models of DNA wrapping up (the trivalents!) in 2009, so we had a lot
more interesting use cases but other places like OpenMM have that already too.  Benchmarks are always important but so
many factors going into one final advertised number.  One compiler for NVIDIA, AMD, and even some random chips in a MacBook.

# Really fun things!

Easier scripts for Linux and Mac

```
./build
./buildmac
```

## OSX

Just run it on Mac with ./buildmac, they have some crazy directions online but I copied it for you because new OSX's won't
even let you just copy things into /System/Library/Frameworks/OpenCL.framework/Headers (they don't trust you?)

Here's the joke benchmark, play with the device number and learn things too, it is not always as simple as simple thing
above on Linux machine with AMD RX580

```
device 0: Intel(R) Core(TM) i9-9980HK CPU @ 2.40GHz
device 1: Intel(R) UHD Graphics 630
device 2: AMD Radeon Pro 5300M Compute Engine
Status: SUCCESS
Results: 
	A[0] = 2
	B[0] = 4
	C[0] = 32768
Mean execution time: 
	1 CPU: 10.2875 sec;
	GPU: 5.16649 sec.
Performance gain: 0.991192x
```


# Happy hacking! :pirate_flag: 

I left the MacOSX x86_64-compiled "mult" executable for anyone with a Mac who just hopes it work and wants to see.

## Questions / Essay Ideas - so if you run it 20 times what do you expect?

