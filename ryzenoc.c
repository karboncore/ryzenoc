/*******************************************************************************

ryzenoc v1.0
a utility for overclocking Ryzen 1000 and 2000 series CPUs on Linux

MIT License

Copyright (c) 2018 D. Murphy

Based on zenstates.py (https://github.com/r4m0n/ZenStates-Linux)

Copyright (c) 2017 Thiago Ramon Gon?alves Montoya

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULLAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Write new value to MSR
int writemsr(uint64_t pos, uint64_t val) {
  FILE *fp ;
  unsigned int i;
  char j[4], path[16] ;
  
  for (i=0;i<32;i=i+1) {
    strcpy(path,"/dev/cpu/");
    sprintf(j,"%d",i);
    strcat(path,j);
    strcat(path,"/msr");
    fp = fopen(path, "wb") ;
    if (! fp) break ;
    fseek(fp, pos, SEEK_SET) ;
    fwrite(&val,sizeof(uint64_t),1,fp);
    fclose(fp) ;
  }
}

// Read current value from MSR
uint64_t readmsr(uint64_t pos) {
  FILE *fp ;
  uint64_t buffer;
  
  fp = fopen("/dev/cpu/0/msr", "rb") ;
  if (! fp) {
    printf("%s\n","error: msr module not loaded (run modprobe msr), or you aren't superuser");
    exit(5);
  }
  fseek(fp, pos, SEEK_SET) ;
  fread(&buffer, sizeof(uint64_t), 1, fp) ;
  fclose(fp) ;
  return(buffer) ;
}

// Set new MSR value
uint64_t setbits(uint64_t val, uint64_t base, uint64_t length, uint64_t _id) {
  return( (val ^ (val & ( ((1ULL << length) - 1) << base) ) ) + (_id << base) );
}

// Print status of a pstate
void pstate2str(uint64_t val) {
  if ( val & 0x8000000000000000) {
    int fid =  val & 0x000000ff ;
    int did = (val & 0x00003f00) >> 8 ;
    int vid = (val & 0x003fc000) >> 14 ;
    float ratio = 2.*fid/did ;
    float vcore = 1.55 - 0.00625 * vid ;
    printf("%s%02x","enabled - FID = ",fid) ;
    printf("%s%02x"," - DID = ",did) ;
    printf("%s%02x"," - VID = ",vid) ;
    printf("%s%2.2f"," - ratio = ",ratio);
    printf("%s%1.5f\n"," - vCore = ",vcore);
  }
  else {
    printf("%s\n","disabled");
  }    
}

// Print help
void help() {
  printf("%s","usage: ryzenoc [-h] [-l] [-p {0,1,2,3,4,5,6,7}] [--enable] [--disable]\n\
               [-f FID] [-d DID] [-v VID] [--c6-enable] [--c6-disable]\n\
\n\
sets P-states for Ryzen processors\n\
\n\
optional arguments:\n\
  -h, --help            show this help message and exit\n\
  -l, --list            List all P-States\n\
  -p {0,1,2,3,4,5,6,7}, --pstate {0,1,2,3,4,5,6,7}\n\
                        P-State to set\n\
  --enable              Enable P-State\n\
  --disable             Disable P-State\n\
  -f FID, --fid FID     FID to set (in hex)\n\
  -d DID, --did DID     DID to set (in hex)\n\
  -v VID, --vid VID     VID to set (in hex)\n\
  --c6-enable           Enable C-State C6\n\
  --c6-disable          Disable C-State C6\n\
  -t, --dryrun          Print output but don't write to CPU\n"
  );
  exit(1);
}

// Entry point
int main(int argc, char *argv[]) {
  unsigned int i, pstate=8, vid=248, fid=0, did=0, opts=0; 
    // opts: 1 list, 2 enable, 4 disable, 8 c6enable, 16 c6disable, 32 dryrun
  uint64_t old, val;
  
// List of pstate addresses
  uint64_t pstates[8] = { 0xC0010064, 0xC0010065, 0xC0010066, 0xC0010067, 0xC0010068, 0xC0010069, 0xC001006A, 0xC001006B} ;

// Parse command line arguments
  if (argc==1) { help(); }
  else {
    for (i=1;i<argc;i=i+1) {
      if (strcmp(argv[i],"-l") == 0 || strcmp(argv[i],"--list") == 0) {
        opts = opts | 1 ;
      }
      else if (strcmp(argv[i],"-v") == 0 || strcmp(argv[i],"--vid") == 0) {
        i=i+1;
        vid=strtoul(argv[i],NULL,16);
      }
      else if (strcmp(argv[i],"-f") == 0 || strcmp(argv[i],"--fid") == 0) {
        i=i+1;
        fid=strtoul(argv[i],NULL,16);
      }
      else if (strcmp(argv[i],"-d") == 0 || strcmp(argv[i],"--did") == 0) {
        i=i+1;
        did=strtoul(argv[i],NULL,16);
      }
      else if (strcmp(argv[i],"--enable") == 0) {
        opts = opts | 2 ;
      }
      else if (strcmp(argv[i],"--disable") == 0) {
        opts = opts | 4 ;
      }
      else if (strcmp(argv[i],"-p") == 0 || strcmp(argv[i],"--pstate") == 0) {
        if (pstate == 8) {
          i=i+1;
          pstate=strtoul(argv[i],NULL,16);
        }
        else {
          printf("%s\n","error: only one pstate can be set at a time");
          exit(4);
        }
      }
      else if (strcmp(argv[i],"--c6-enable") == 0) {
        opts = opts | 8 ;
      }
      else if (strcmp(argv[i],"--c6-disable") == 0) {
        opts = opts | 16 ;
      }
      else if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"--help") == 0) {
        help();
      }
      else if (strcmp(argv[i],"-t") == 0 || strcmp(argv[i],"--dryrun") == 0) {
        opts = opts | 32 ;
      }
      else { 
        printf("%s%s\n","error: invalid option ",argv[i]); 
        exit(2);
      }
    }
  }
  
// Write values to pstate
  if (pstate < 8){
    val = readmsr(pstates[pstate]);
    old = val;
    printf("%s%u: ","current P",pstate);
    pstate2str(old);

    if (opts & 2) {
      if (vid < 248 && fid > 0 && did > 0) {
        val = setbits(val, 63, 1, 1);
        printf("%s\n","enabling state");
      }
      else {
        printf("%s\n","error: fid, did, and vid must be supplied to enable new pstate");
        exit(3);
      }
    }
    else if (opts & 4){
      val = setbits(val, 63, 1, 0);
      printf("%s\n","disabling state");
    }    
    if (vid < 248) {
      val = setbits(val,14,8,vid) ;
      printf("%s%x\n","setting VID to ",vid);
    }
    if (fid > 0) {
      val = setbits(val,0,8,fid) ;
      printf("%s%x\n","setting FID to ",fid);
    }
    if (did > 0) {
      val = setbits(val,8,6,did) ;
      printf("%s%x\n","setting DID to ",did);
    }
    
    if (val != old) {
      if (! (opts & 32)) {
        if ( ! readmsr(0xC0010015) & 0x00200000) {
          printf("%s\n","locking TSC frequency");     
          writemsr(0xC0010015, readmsr(0xC0010015) | 0x00200000);
        }
        writemsr(pstates[pstate],val) ;
      }
      printf("%s%u: ","new P",pstate);
      pstate2str(val);
    }
  }
  
// Enable or disable c6 state
  if (opts & 8){
    if (! (opts & 32)) {
      writemsr(0xC0010292, readmsr(0xC0010292) | 0x0000000100000000) ;
      writemsr(0xC0010296, readmsr(0xC0010296) | 0x00404040) ;
    }
    printf("%s\n","enabling C6 state");
  }
  else if (opts & 16){
    if (! (opts & 32)) {
      writemsr(0xC0010292, readmsr(0xC0010292) & ~ 0x0000000100000000) ;
      writemsr(0xC0010296, readmsr(0xC0010296) & ~ 0x00404040) ;
    }
    printf("%s\n","disabling C6 state");
  }
  
  if (opts & 32) { printf("%s\n","*** dry run, values were not written ***"); }
  
// List all states and their current status
  if (opts & 1) {
    for ( i=0; i<8; i=i+1) {
      val = readmsr(pstates[i]);
      printf("P%u",i) ;
      printf("%s"," - ");
      pstate2str(val) ;
    }
    val = readmsr(0xC0010292);
    printf("%s","C6 state - package - ");
    if ( val & 0x0000000100000000 ) {
      printf("%s\n","enabled");
    }
    else {
      printf("%s\n","disabled");
    }
    val = readmsr(0xC0010296);
    printf("%s","C6 state - core - ");
    if ( ( val & 0x00404040 ) == 0x00404040 ) {
      printf("%s\n","enabled");
    }
    else {
      printf("%s\n","disabled");
    }
  }
}
