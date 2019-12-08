# ryzenoc

A utility for overclocking Ryzen 1000 and 2000 series CPUs on Linux, written in C.

Based on zenstates.py (https://github.com/r4m0n/ZenStates-Linux), but without the python dependency.

### Usage:

    ryzenoc [-h] [-l] [-p {0,1,2,3,4,5,6,7}] [--enable] [--disable]
                  [-f FID] [-d DID] [-v VID] [--c6-enable] [--c6-disable]

    sets P-states for Ryzen processors

    optional arguments:
      -h, --help            show this help message and exit
      -l, --list            List all P-States
      -p {0,1,2,3,4,5,6,7}, --pstate {0,1,2,3,4,5,6,7}
                            P-State to set
      --enable              Enable P-State
      --disable             Disable P-State
      -f FID, --fid FID     FID to set (in hex)
      -d DID, --did DID     DID to set (in hex)
      -v VID, --vid VID     VID to set (in hex)
      --c6-enable           Enable C-State C6
      --c6-disable          Disable C-State C6
      -t, --dryrun          Print output but don't write to CPU
    
### To compile:

  gcc ryzenoc.c -O2 -o ryzenoc
  
### PS:

  If you try this on a Ryzen 3000 series CPU, let me know if it works!
