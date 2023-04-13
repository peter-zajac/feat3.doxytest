#!/usr/bin/env python
# FEAT3: Finite Element Analysis Toolbox, Version 3
# Copyright (C) 2010 - 2023 by Stefan Turek & the FEAT group
# FEAT3 is released under the GNU General Public License version 3,
# see the file 'copyright.txt' in the top level directory for details.
########################################################################################################################
# DFG95 flow-around-a-cylinder benchmark 3 post-processing script
# ---------------------------------------------------------------
#
# USAGE: bench3plots.py <outprefix> <logfiles...>
#
# This script analyses a (non-empty) sequence of output logfiles generated by the dfg95-ccnd-unsteady application
# used to solve the 'bench 3' configuration by computing various quantities required by the benchmark setup, e.g.
# minimum and maximum drag/lift forces, as well as their EOCs. In addition to that, this script also generates a batch
# of Matplotlib PyPlot scripts which plot the various quantities over the whole simulation time. All plot files as well
# as the analysis results are written to individual output files, whose filenames are prefixed by the <outprefix> given
# as the first command line argument to this script.
#
# Typically, one want to analyse the convergence behavior of several simulation runs which only differ on the mesh
# refinement level and/or time-step size. Assuming appropriate naming of the log files, the call
#
# > bench3plots.py k=400_levels lvl=3_k=400.log lvl=4_k=400.log lvl=5_k=400.log lvl=6_k=400.log
#
# would analyse the convergence for various mesh refinement levels (3,4,5,6) for the same time step size k=1/400,
# whereas the call
#
# > bench3plots.py lvl=5_tsteps lvl=5_k=100.log lvl=5_k=200.log lvl=5_k=400.log lvl=5_k=800.log
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
      fo.write("%g,\n" % vt[i])
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
      fo.write("%g,\n" % vt[i])
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

def write_table(fo, data, j, name):
  fo.write("\nAnalysis of %s values\n" % name)
  fo.write(" T-Max             Max               Max-EOC          End               End-EOC          Name\n")
  for i in range(0,len(data)):
    d = data[i]
    xd = d[j]
    t_max = 0.0
    x_max = 0.0
    x_end = xd[-1]
    max_eoc = 0.0
    end_eoc = 0.0
    for k in range(0, len(xd)):
      if(xd[k] > x_max):
        x_max = xd[k]
        t_max = d[1][k]
    if i > 1:
      max_eoc = eoc(x_max, max(data[i-1][j]), max(data[i-2][j]))
      end_eoc = eoc(x_end, data[i-1][j][-1], data[i-2][j][-1])
    fo.write("%15.12f   %15.12f   %15.12f   %15.12f   %15.12f   %s\n" % (t_max, x_max, max_eoc, x_end, end_eoc, d[0]))


########################################################################################################################
########################################################################################################################
# MAIN
########################################################################################################################
########################################################################################################################

if len(sys.argv) < 3:
  print("USAGE: bench3plots.py <outprefix> <logfiles...>")
  sys.exit(0)

prefix = sys.argv[1]

data = []

# loop over all input log files
for fn in sys.argv[2:]:
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
    c_t = float(r.group(1))
    if(c_t > 8.0):
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
  # set name
  if fn.endswith(".log"):
    name = fn[:-4]
  else:
    name = fn
  data += [[name,x_t,x_dl,x_dv,x_ll,x_lv,x_pd,x_fu,x_fl,x_h0,x_h1,x_vo,x_di]]

# generate plot files
genplot_full("%s_drag_l_full.py" % prefix, data, 2, "Drag (Line)")
genplot_zoom("%s_drag_l_zoom.py" % prefix, data, 2, "Drag (Line)", 3.5, 4.3, 2.85, 3.0)
genplot_full("%s_drag_v_full.py" % prefix, data, 3, "Drag (Volume)")
genplot_zoom("%s_drag_v_zoom.py" % prefix, data, 3, "Drag (Volume)", 3.5, 4.3, 2.85, 3.0)
genplot_zoom("%s_drag_v_zoom_2.py" % prefix, data, 3, "Drag (Volume)", 5.2, 5.6, 2, 2.5)
genplot_full("%s_lift_l_full.py" % prefix, data, 4, "Lift (Line)")
genplot_zoom("%s_lift_l_zoom.py" % prefix, data, 4, "Lift (Line)", 5.65, 5.75, 0.4, 0.5)
genplot_full("%s_lift_v_full.py" % prefix, data, 5, "Lift (Volume)")
genplot_zoom("%s_lift_v_zoom.py" % prefix, data, 5, "Lift (Volume)", 5.65, 5.75, 0.4, 0.5)
genplot_zoom("%s_lift_v_zoom_2.py" % prefix, data, 5, "Lift (Volume)", 7.5, 8.0, -0.1, 0.05)
genplot_full("%s_pdiff_full.py"  % prefix, data, 6, "Pressure Difference")
genplot_zoom("%s_pdiff_zoom.py"  % prefix, data, 6, "Pressure Difference", 3.6, 4.3, 2.28, 2.34)
genplot_full("%s_flux_u_full.py" % prefix, data, 7, "Upper Flux")
genplot_full("%s_flux_l_full.py" % prefix, data, 8, "Lower Flux")
genplot_full("%s_h0_norm_u.py"   % prefix, data, 9, "H0-Norm of U")
genplot_full("%s_h1_norm_u.py"   % prefix, data,10, "H1-Norm of U")
genplot_full("%s_vorticity.py"   % prefix, data,11, "Vorticity")

# analyse benchmark values
print("Writing '%s_analysis.txt'..." % prefix)
fo = open("%s_analysis.txt" % prefix, "wt")
write_table(fo, data, 2, "Drag (Line)")
write_table(fo, data, 3, "Drag (Volume)")
write_table(fo, data, 4, "Lift (Line)")
write_table(fo, data, 5, "Lift (Volume)")
write_table(fo, data, 6, "Pressure Difference")
write_table(fo, data, 7, "Upper Flux")
write_table(fo, data, 8, "Lower Flux")
write_table(fo, data, 9, "H0-Norm of U")
write_table(fo, data,10, "H1-Norm of U")
write_table(fo, data,11, "Vorticity")
write_table(fo, data,12, "Divergence")
fo.close()
