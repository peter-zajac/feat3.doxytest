#!/usr/bin/env python
# vim: set filetype=python sw=2 sts=2 et nofoldenable :
# This is a python script.
# If you encounter problemes when executing it on its own, start it with a python interpreter
import subprocess
import sys
import os
import time

sys.dont_write_bytecode = True

for i in range(1, 25):
  with open("temp", "w") as f:
    f.write("#!/bin/bash" + os.linesep)
    f.write("#PBS -q short" + os.linesep)
    f.write("#PBS -l vmem=15000mb,nodes=" + str(i) + ":ppn=4:ib" + os.linesep)
    f.write("#PBS -l walltime=01:00:00" + os.linesep)
    f.write("#PBS -N feat" + os.linesep)
    f.write("#PBS -j oe" + os.linesep)

    f.write("export OMP_NUM_THREADS=1" + os.linesep)
    f.write("source ~/.bashrc &>/dev/null" + os.linesep)
    f.write("mpirun -np " + str(i*4) + " --map-by node ~/feat/trunk/applications/poisson_dirichlet_2d  --level " +  str(9) + " 4 --part_min_elems 500" + os.linesep)

  #sbatch
  subprocess.call(["qsub", "temp"])
  time.sleep(1)

os.remove("temp")
