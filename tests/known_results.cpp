/* Copyright (C) 2003 Massachusetts Institute of Technology  
%
%  This program is free software; you can redistribute it and/or modify
%  it under the terms of the GNU General Public License as published by
%  the Free Software Foundation; either version 2, or (at your option)
%  any later version.
%
%  This program is distributed in the hope that it will be useful,
%  but WITHOUT ANY WARRANTY; without even the implied warranty of
%  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%  GNU General Public License for more details.
%
%  You should have received a copy of the GNU General Public License
%  along with this program; if not, write to the Free Software Foundation,
%  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "dactyl.h"

double one(const vec &) { return 1.0; }

double rods(const vec &r) {
  vec p = r;
  while (p.x() < -0.5) p.set_direction(X, p.x() + 1.0);
  while (p.x() >  0.5) p.set_direction(X, p.x() - 1.0);
  while (p.y() < -0.5) p.set_direction(Y, p.y() + 1.0);
  while (p.y() >  0.5) p.set_direction(Y, p.y() - 1.0);
  if (p.x()*p.x() + p.y()*p.y() < 0.3) return 12.0;
  return 1.0;
}

void compare(double a, double b, const char *n) {
  if (fabs(a-b) > fabs(b)*1e-5) {
    master_printf("Differs by\t%lg out of\t%lg\n", a-b, b);
    master_printf("This gives a fractional error of %lg\n", fabs(a-b)/fabs(b));
    abort("Error in %s\n", n);
  } else {
    master_printf("Passed %s\n", n);
  }
}

double using_pml_ez(const volume &v, double eps(const vec &)) {
  const double ttot = 30.0;
  mat ma(v, eps);
  ma.use_pml_everywhere(1.0);
  fields f(&ma);
  f.add_point_source(Ez, 0.2, 3.0, 0.0, 2.0, v.center());
  while (f.time() < ttot) f.step();
  monitor_point p;
  f.get_point(&p, v.center());
  return real(p.get_component(Ez));
}

double x_periodic_y_pml(const volume &v, double eps(const vec &)) {
  const double ttot = 30.0;
  mat ma(v, eps);
  ma.use_pml(Y, High, 1.0);
  ma.use_pml(Y, Low, 1.0);
  fields f(&ma);
  f.add_point_source(Ez, 0.2, 3.0, 0.0, 2.0, v.center());
  f.use_metal_everywhere();
  f.use_bloch(X, 0.1);
  while (f.time() < ttot) f.step();
  monitor_point p;
  f.get_point(&p, v.center());
  return real(p.get_component(Ez));
}

double x_periodic(const volume &v, double eps(const vec &)) {
  const double ttot = 30.0;
  mat ma(v, eps);
  fields f(&ma);
  f.add_point_source(Ez, 0.2, 3.0, 0.0, 2.0, v.center());
  f.use_metal_everywhere();
  f.use_bloch(X, 0.1);
  while (f.time() < ttot) f.step();
  monitor_point p;
  f.get_point(&p, v.center());
  return real(p.get_component(Ez));
}

double periodic_ez(const volume &v, double eps(const vec &)) {
  const double ttot = 30.0;
  mat ma(v, eps);
  fields f(&ma);
  f.add_point_source(Ez, 0.2, 3.0, 0.0, 2.0, v.center());
  vec k;
  switch (v.dim) {
  case D1: k = vec(0.3); break;
  case D2: k = vec2d(0.3,0.4); break;
  case D3: k = vec(0.3,0.5,0.8); break;
  case Dcyl: k = vec(0.3,0.2); break;
  }
  f.use_bloch(k);
  while (f.time() < ttot) f.step();
  monitor_point p;
  f.get_point(&p, v.center());
  return real(p.get_component(Ez));
}

double metallic_ez(const volume &v, double eps(const vec &)) {
  const double ttot = 10.0;
  mat ma(v, eps);
  fields f(&ma);
  f.add_point_source(Ez, 0.2, 3.0, 0.0, 2.0, v.center());
  f.use_metal_everywhere();
  while (f.time() < ttot) f.step();
  monitor_point p;
  f.get_point(&p, v.center());
  return real(p.get_component(Ez));
}

int main(int argc, char **argv) {
  initialize mpi(argc, argv);
  const char *dirname = "known_results-out";
  trash_output_directory(dirname);
  master_printf("Testing with some known results...\n");
  const double a = 10.0;

  compare(0.294573, metallic_ez(voltwo(1.0, 1.0, a), one),
          "1x1 metallic 2D TM");
  compare(0.816553, using_pml_ez(voltwo(3.0, 3.0, a), one),
          "1x1 PML 2D TM");
  compare(-0.0443493, x_periodic(voltwo(1.0, 1.0, a), one),
          "1x1 X periodic 2D TM");
  compare(0.139809, periodic_ez(voltwo(1.0, 3.0, a), rods),
          "1x1 fully periodic 2D TM rods");
  compare(0.333574, periodic_ez(voltwo(1.0, 3.0, a), one),
          "1x1 fully periodic 2D TM");
  compare(1.25203, x_periodic_y_pml(voltwo(1.0, 2.0, a), one),
          "1x1 X periodic Y PML 2D TM");
  compare(-26.3698, metallic_ez(vol3d(1.0, 1.0, 1.0, a), one),
          "1x1x1 metallic 3D");
  compare(-222.593, x_periodic(vol3d(1.0, 1.0, 1.0, a), one),
          "1x1x1 X periodic 3D");
  compare(-210.372, x_periodic_y_pml(vol3d(1.0, 2.0, 1.0, a), one),
          "1x1x1 X periodic Y PML 3D");
  compare(-178.414, periodic_ez(vol3d(1.0, 1.0, 1.0, a), rods),
          "1x1x1 fully periodic 3D rods");
  compare(-223.131, periodic_ez(vol3d(1.0, 1.0, 1.0, a), one),
          "1x1x1 fully periodic 3D");

  return 0;
}
