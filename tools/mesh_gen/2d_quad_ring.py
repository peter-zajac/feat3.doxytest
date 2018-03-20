#!/usr/bin/env python
# -*- coding: utf-8 -*-
# This tool generates a 2D ring mesh.
# The generated meshfile also contains two meshparts representing
# the two boundaries of the ring domain named:
# bnd:i     The inner boundary ring meshpart
# bnd:o     The outer boundary ring meshpart
#
# USAGE: 2d_ring_quad.py cx cy r0 r1 m n filename
#
# Options:
# cx cy     The coordinates of the ring barycentre
# r0        The inner radius of the ring
# r1        The outer radius of the ring
# m         The number of angle subdivisions; must be > 2
# n         The number of radius subdivisions
# filename  The name of the meshfile to be created
#
# Note: This script is compatible with both Python 2.x and Python 3.x
#
# \author Peter Zajac
#
import os.path
import sys
import math

# write ring meshpart
def mp_ring(f, name, i, m, n):
  f.write("  <MeshPart name=\"" + name + "\" parent=\"root\" topology=\"none\" size=\"%i %i\">\n" % (m,m))
  f.write("    <Mapping dim=\"0\">\n")
  for j in range(0, m):
    f.write("      %i\n" % (i*m + j))
  f.write("    </Mapping>\n")
  f.write("    <Mapping dim=\"1\">\n")
  for j in range(0, m):
    f.write("      %i\n" % (i*m + j))
  f.write("    </Mapping>\n")
  f.write("  </MeshPart>\n")

####################################################################################################
####################################################################################################
####################################################################################################

# get the script filename
myself = os.path.basename(sys.argv[0])

# do we have enough arguments?
if len(sys.argv) < 8:
  print("")
  print("USAGE: " + myself + " cx cy r0 r1 m n filename")
  print("")
  print("Options:")
  print("cx cy     The coordinates of the ring barycentre")
  print("r0        The inner radius of the ring")
  print("r1        The outer radius of the ring")
  print("m         The number of angle subdivisions; must be > 2")
  print("n         The number of radius subdivisions")
  print("filename  The name of the meshfile to be created")
  print("")
  print("Information:")
  print("This tool generates a 2D ring mesh.")
  print("The generated meshfile also contains two meshparts representing")
  print("the two boundaries of the ring domain named:")
  print("bnd:i     The inner boundary ring meshpart")
  print("bnd:o     The outer boundary ring meshpart")
  sys.exit(0)

# parse input arguments
cx = float(sys.argv[1])
cy = float(sys.argv[2])
r0 = float(sys.argv[3])
r1 = float(sys.argv[4])
m  = int(sys.argv[5])
n  = int(sys.argv[6])
filename = sys.argv[7]

# some basic sanity checks
if(r0 <= 1E-8):
  print("ERROR: invalid inner radius: r0 must be > 0")
  sys.exit(1)
if(r1 <= r0 + 1E-5):
  print("ERROR: invalid outer radius: r1 must be > r0")
  sys.exit(1)
if(m <= 2):
  print("ERROR: 'm' must be greater than 2")
  sys.exit(1)
if(n <= 0):
  print("ERROR: 'n' must be positive")
  sys.exit(1)

# compute vertex, edge and quad counts
nv = m*(n+1)
ne = m*(n+1) + n*m
nq = m*n

# print some basic information
print("Slices: %i x %i" % (m, n))
print("Verts.: %i" % nv)
print("Edges.: %i" % ne)
print("Quads.: %i" % nq)
print("")

# try to create the output file
print("Creating '" + filename + "'...")
f = open(filename, "wt")
# write header
f.write("<FeatMeshFile version=\"1\" mesh=\"conformal:hypercube:2:2\">\n")
f.write("  <!-- Generated by the FEAT '" + myself + "' tool -->\n")
# write tool call as comment
f.write("  <!-- call: " + myself)
for i in range(1,8):
  f.write(" " + sys.argv[i])
f.write(" -->\n")
# write mesh
f.write("  <Mesh type=\"conformal:hypercube:2:2\" size=\"%i %i %i\">\n" % (nv, ne, nq))
f.write("    <Vertices>\n")
# write vertices in ring-wise order
for i in range(0,n+1):
  r = r0 + (r1-r0)*(float(i)/float(n))
  for j in range(0,m):
    t = 2.0*math.pi*float(j)/float(m)
    f.write("      %g %g\n" % (cx+r*math.cos(t), cy+r*math.sin(t)))
f.write("    </Vertices>\n")
f.write("    <Topology dim=\"1\">\n")
# write ring edges (i.e. endpoints have same radius)
for i in range(0,n+1):
  for j in range(0,m-1):
    f.write("      %i %i\n" % (i*m+j, i*m+j+1))
  f.write(  "      %i %i\n" % (i*m+m-1, i*m))
# write angle edges (i.e. endpoints have same angle)
for j in range(0,m):
  for i in range(0,n):
    f.write("      %i %i\n" % (i*m+j, (i+1)*m+j))
f.write("    </Topology>\n")
f.write("    <Topology dim=\"2\">\n")
for i in range(0,n):
  for j in range(0,m-1):
    f.write("      %i %i %i %i\n" % (i*m+j, i*m+j+1, (i+1)*m+j, (i+1)*m+j+1))
  f.write(  "      %i %i %i %i\n" % (i*m+m-1, i*m    , (i+1)*m+m-1, (i+1)*m))
f.write("    </Topology>\n")
f.write("  </Mesh>\n")
# write inner meshpart
mp_ring(f, "bnd:i", 0, m, n)
# write outer meshpart
mp_ring(f, "bnd:o", n, m, n)
f.write("</FeatMeshFile>\n")

print("Done!")
