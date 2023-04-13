#!/usr/bin/env python
# FEAT3: Finite Element Analysis Toolbox, Version 3
# Copyright (C) 2010 - 2023 by Stefan Turek & the FEAT group
# FEAT3 is released under the GNU General Public License version 3,
# see the file 'copyright.txt' in the top level directory for details.
########################################################################################################################
# DFG95 flow-around-a-cylinder benchmark 2 post-processing script
# ---------------------------------------------------------------
#
# USAGE: bench3plots.py <T_min> <outprefix> <logfiles...>
#
# This script analyses a (non-empty) sequence of output logfiles generated by the dfg95-ccnd-unsteady application
# used to solve the 'bench 2' configuration by computing various quantities required by the benchmark setup, e.g.
# the periodic cycle length and minimum/maximum drag/lift forces, as well as their EOCs. In addition to that, this
# script also generates a batch of Matplotlib PyPlot scripts which plot the various quantities over the whole periodic
# cyle time. All plot files as well as the analysis results are written to individual output files, whose filenames are
# prefixed by the <outprefix> given as the second command line argument to this script. The first argument T_min
# specifies at which time the script should start analysing the lift values to find a matching periodic cycle.
# Usually, the total simulation time is the range [0, 30] and a cycle is chosen from either the range [20,30] or even
# from [25,30], which is specified by setting T_min to 20 or 25, respectively.
#
# Typically, one want to analyse the convergence behavior of several simulation runs which only differ on the mesh
# refinement level and/or time-step size. Assuming appropriate naming of the log files, the call
#
# > bench2plots.py 20 k=400_levels lvl=3_k=400.log lvl=4_k=400.log lvl=5_k=400.log lvl=6_k=400.log
#
# would analyse the convergence for various mesh refinement levels (3,4,5,6) for the same time step size k=1/400,
# whereas the call
#
# > bench2plots.py 20 lvl=5_tsteps lvl=5_k=100.log lvl=5_k=200.log lvl=5_k=400.log lvl=5_k=800.log
#
# would analyse the convergence for various time step sizes (k=1/100,k=1/200,k=1/400,k=1/800) on the same mesh.
#
# \author Peter Zajac
########################################################################################################################
import os
import sys
import re
import math

colorlist = ['red','green','blue','black','purple','orange','pink']

def eoc(x0, x1, x2):
  num = abs(x2-x1)
  den = abs(x1-x0)
  if (num < 1E-15) or (den < 1E-15):
    return 0.0
  else:
    return  math.log2(num/den)

def genplot_full(fn, data, j, yname):
  print("Writing '%s'..." % fn)
  fo = open(fn, "wt")
  fo.write("import matplotlib.pyplot as plt\n")
  # write all data entries
  names = ""
  for k in range(0,len(data)):
    v = data[k]
    if k > 0:
      names += ","
    names += "'" + v[0] + "'"
    vt = v[1]
    vx = v[j]
    fo.write("t_%i = [\n" % (k))
    for i in range(0,len(vt)):
      fo.write("%g,\n" % (vt[i] - vt[0])) # time relative to first time step
    fo.write("]\n")
    fo.write("x_%i = [\n" % (k))
    for i in range(0,len(vt)):
      fo.write("%g,\n" % vx[i])
    fo.write("]\n")
    fo.write("plt.plot(t_%i,x_%i,color='%s')\n" % (k,k,colorlist[k]))
  fo.write("plt.xlabel('Cycle Time')\n")
  fo.write("plt.ylabel('%s')\n" % yname)
  fo.write("plt.legend((%s))\n" % names)
  fo.write("plt.grid(True, 'major', 'both', linestyle='--')\n")
  fo.write("plt.show()\n")
  fo.close()

def genplot_zoom(fn, data, j, yname, x0, x1, y0, y1):
  print("Writing '%s'..." % fn)
  fo = open(fn, "wt")
  fo.write("import matplotlib.pyplot as plt\n")
  names = ""
  # write all data entries
  for k in range(0,len(data)):
    v = data[k]
    if k > 0:
      names += ","
    names += "'" + v[0] + "'"
    vt = v[1]
    vx = v[j]
    fo.write("t_%i = [\n" % (k))
    for i in range(0,len(vt)):
      fo.write("%g,\n" % (vt[i] - vt[0])) # time relative to first time step
    fo.write("]\n")
    fo.write("x_%i = [\n" % (k))
    for i in range(0,len(vt)):
      fo.write("%g,\n" % vx[i])
    fo.write("]\n")
    fo.write("plt.plot(t_%i,x_%i,color='%s')\n" % (k,k,colorlist[k]))
  fo.write("plt.xlabel('Cycle Time')\n")
  fo.write("plt.xlim(%g,%g)\n" % (x0,x1))
  fo.write("plt.ylabel('%s')\n" % (yname))
  fo.write("plt.ylim(%g,%g)\n" % (y0,y1))
  fo.write("plt.legend((%s))\n" % names)
  fo.write("plt.grid(True, 'major', 'both', linestyle='--')\n")
  fo.write("plt.show()\n")
  fo.close()

def write_table1(fo, data, j, name):
  fo.write("\nAnalysis of %s values\n" % name)
  fo.write(" Min               Max               Mean              Amp              Name\n")
  for d in data:
    xd = d[j]
    x_min = min(xd)
    x_max = max(xd)
    fo.write("%15.12f   %15.12f   %15.12f   %15.12f   %s\n" % (x_min, x_max, 0.5*(x_min+x_max), x_max-x_min, d[0]))
  fo.write(" Min-EOC           Max-EOC           Mean-EOC          Amp-EOC\n")
  for i in range(2,len(data)):
    d0 = data[i  ][j]
    d1 = data[i-1][j]
    d2 = data[i-2][j]
    x_min0 = min(d0)
    x_max0 = max(d0)
    x_min1 = min(d1)
    x_max1 = max(d1)
    x_min2 = min(d2)
    x_max2 = max(d2)
    x_mean0 = 0.5*(x_max0+x_min0)
    x_mean1 = 0.5*(x_max1+x_min1)
    x_mean2 = 0.5*(x_max2+x_min2)
    x_amp0 = x_max0 - x_min0
    x_amp1 = x_max1 - x_min1
    x_amp2 = x_max2 - x_min2
    eoc_min = eoc(x_min0, x_min1, x_min2)
    eoc_max = eoc(x_max0, x_max1, x_max2)
    eoc_mean = eoc(x_mean0, x_mean1, x_mean2)
    eoc_amp = eoc(x_amp0, x_amp1, x_amp2)
    fo.write("%15.12f   %15.12f   %15.12f   %15.12f\n" % (eoc_min, eoc_max, eoc_mean, eoc_amp))

def write_table2(fo, data, j, name):
  fo.write("\nAnalysis of %s values\n" % name)
  fo.write(" Min #1            Max #1            Min #2            Max #2            Mean              Amp              Name\n")
  for d in data:
    xd = d[j]
    l = int(len(xd)/2)
    x_min_1 = min(xd[0:l])
    x_max_1 = max(xd[0:l])
    x_min_2 = min(xd[l:])
    x_max_2 = max(xd[l:])
    x_min = min(x_min_1, x_min_1)
    x_max = max(x_max_1, x_max_2)
    fo.write("%15.12f   %15.12f   %15.12f   %15.12f   %15.12f   %15.12f   %s\n" % (x_min_1, x_max_1, x_min_2, x_max_2, 0.5*(x_min+x_max), x_max-x_min, d[0]))
  fo.write(" Min-EOC #1        Max-EOC #1        Min-EOC #2        Max-EOC #2        Mean-EOC          Amp-EOC\n")
  for i in range(2,len(data)):
    d0 = data[i  ][j]
    d1 = data[i-1][j]
    d2 = data[i-2][j]
    l0 = int(len(d0)/2)
    l1 = int(len(d1)/2)
    l2 = int(len(d2)/2)
    x_min0_1 = min(d0[0:l0])
    x_max0_1 = max(d0[0:l0])
    x_min1_1 = min(d1[0:l1])
    x_max1_1 = max(d1[0:l1])
    x_min2_1 = min(d2[0:l2])
    x_max2_1 = max(d2[0:l2])
    x_min0_2 = min(d0[l0:])
    x_max0_2 = max(d0[l0:])
    x_min1_2 = min(d1[l1:])
    x_max1_2 = max(d1[l1:])
    x_min2_2 = min(d2[l2:])
    x_max2_2 = max(d2[l2:])
    x_min0 = min(x_min0_1, x_min0_1)
    x_max0 = max(x_max0_1, x_max0_2)
    x_min1 = min(x_min1_1, x_min1_1)
    x_max1 = max(x_max1_1, x_max1_2)
    x_min2 = min(x_min2_1, x_min2_1)
    x_max2 = max(x_max2_1, x_max2_2)
    x_mean0 = 0.5*(x_max0+x_min0)
    x_mean1 = 0.5*(x_max1+x_min1)
    x_mean2 = 0.5*(x_max2+x_min2)
    x_amp0 = x_max0 - x_min0
    x_amp1 = x_max1 - x_min1
    x_amp2 = x_max2 - x_min2
    eoc_min_1 = eoc(x_min0_1, x_min1_1, x_min2_1)
    eoc_max_1 = eoc(x_max0_1, x_max1_1, x_max2_1)
    eoc_min_2 = eoc(x_min0_2, x_min1_2, x_min2_2)
    eoc_max_2 = eoc(x_max0_2, x_max1_2, x_max2_2)
    eoc_mean = eoc(x_mean0, x_mean1, x_mean2)
    eoc_amp = eoc(x_amp0, x_amp1, x_amp2)
    fo.write("%15.12f   %15.12f   %15.12f   %15.12f   %15.12f   %15.12f\n" % (eoc_min_1, eoc_max_1, eoc_min_2, eoc_max_2, eoc_mean, eoc_amp))

########################################################################################################################
########################################################################################################################
# MAIN
########################################################################################################################
########################################################################################################################

if len(sys.argv) < 4:
  print("USAGE: bench2plots.py <T_min> <outprefix> <logfiles...>")
  sys.exit(0)

T_min = float(sys.argv[1])
#T_max = float(sys.argv[2])
prefix = sys.argv[2]

data = []

# loop over all input log files
for fn in sys.argv[3:]:
  print("Parsing input log file '%s'..." % fn)
  fi = open(fn, "rt")
  x_t  = [] # time
  x_dl = [] # drag line
  x_dv = [] # drag vol
  x_ll = [] # lift line
  x_lv = [] # lift vol
  x_pd = [] # pressure diff
  x_fu = [] # upper flux
  x_fl = [] # lower flux
  x_h0 = [] # H0-norm of u
  x_h1 = [] # H1-norm of u
  x_vo = [] # vorticity
  x_di = [] # divergence
  for l in fi:
    l = l.strip()
    r = re.match("\s*\d+:\s*([\d\.]+)\s+>", l)
    if not r:
      continue
    # check time-stamp
    c_t = float(r.group(1))
    if (c_t < T_min):# or (c_t > T_max):
      continue
    r = re.match(".+> Runtime", l)
    if r:
     x_t  = x_t  + [c_t]
     continue
    r = re.match(".+> DC/LC/PD:\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)", l)
    if r:
      x_dl += [float(r.group(1))]
      x_dv += [float(r.group(2))]
      x_ll += [float(r.group(3))]
      x_lv += [float(r.group(4))]
      x_pd += [float(r.group(5))]
      continue
    r = re.match(".+> FX/H0/H1:\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)", l)
    if r:
      x_fu += [float(r.group(1))]
      x_fl += [float(r.group(2))]
      x_h0 += [float(r.group(3))]
      x_h1 += [float(r.group(4))]
      continue
    r = re.match(".+> VC/DV\.\.\.:\s+(\S+)\s+(\S+)", l)
    if r:
      x_vo += [float(r.group(1))]
      x_di += [float(r.group(2))]
      continue
  fi.close()
  # determine cycles
  cyc = []
  cerr = []
  for i in range(1, len(x_ll)-1):
    if (x_ll[i] < x_ll[i-1]) and (x_ll[i] < x_ll[i+1]):
      cyc += [i]
      cerr += [abs((x_ll[i-1] - x_ll[i]) - (x_ll[i+1] - x_ll[i]))]
  print("Found %i cycles in time range from %f to %f" % (len(cyc)-1, T_min, x_t[-1]))
  # choose cycle based on best symmetry
  ib = -1
  ie = -1
  cm = 1E+99
  for j in range(0, len(cyc)-1):
    if cerr[j] < cm:
      ib = cyc[j]
      ie = cyc[j+1]
      cm = cerr[j]
  print("Chose cycle at %f with length %f and lift symmetry error %g" % (x_t[ib], x_t[ie]-x_t[ib], cm))
  # set name
  if fn.endswith(".log"):
    name = fn[:-4]
  else:
    name = fn
  data += [[name,x_t[ib:ie],x_dl[ib:ie],x_dv[ib:ie],x_ll[ib:ie],x_lv[ib:ie],x_pd[ib:ie],
            x_fu[ib:ie],x_fl[ib:ie],x_h0[ib:ie],x_h1[ib:ie],x_vo[ib:ie],x_di[ib:ie]]]

# generate plot files
genplot_full("%s_drag_l_full.py" % prefix, data, 2, "Drag (Line)")
genplot_zoom("%s_drag_l_zoom.py" % prefix, data, 2, "Drag (Line)", 0.13, 0.24, 3.17, 3.24)
genplot_full("%s_drag_v_full.py" % prefix, data, 3, "Drag (Volume)")
genplot_zoom("%s_drag_v_zoom.py" % prefix, data, 3, "Drag (Volume)", 0.13, 0.24, 3.17, 3.24)
genplot_full("%s_lift_l_full.py" % prefix, data, 4, "Lift (Line)")
genplot_zoom("%s_lift_l_zoom.py" % prefix, data, 4, "Lift (Line)", 0.14, 0.19, 0.91, 1.0)
genplot_full("%s_lift_v_full.py" % prefix, data, 5, "Lift (Volume)")
genplot_zoom("%s_lift_v_zoom.py" % prefix, data, 5, "Lift (Volume)", 0.14, 0.19, 0.91, 1.0)
genplot_full("%s_pdiff_full.py"  % prefix, data, 6, "Pressure Difference")
genplot_zoom("%s_pdiff_zoom.py"  % prefix, data, 6, "Pressure Difference", 0.15, 0.24, 2.45, 2.52)
genplot_full("%s_flux_u_full.py" % prefix, data, 7, "Upper Flux")
genplot_full("%s_flux_l_full.py" % prefix, data, 8, "Lower Flux")
genplot_full("%s_h0_norm_u.py"   % prefix, data, 9, "H0-Norm of U")
genplot_full("%s_h1_norm_u.py"   % prefix, data,10, "H1-Norm of U")
genplot_full("%s_vorticity.py"   % prefix, data,11, "Vorticity")

# analyse benchmark values
print("Writing '%s_analysis.txt'..." % prefix)
fo = open("%s_analysis.txt" % prefix, "wt")
write_table2(fo, data, 2, "Drag (Line)")
write_table2(fo, data, 3, "Drag (Volume)")
write_table1(fo, data, 4, "Lift (Line)")
write_table1(fo, data, 5, "Lift (Volume)")
write_table2(fo, data, 6, "Pressure Difference")
write_table2(fo, data, 7, "Upper Flux")
write_table2(fo, data, 8, "Lower Flux")
write_table1(fo, data, 9, "H0-Norm of U")
write_table1(fo, data,10, "H1-Norm of U")
write_table1(fo, data,11, "Vorticity")
write_table1(fo, data,12, "Divergence")
fo.close()
