<?php

/**
 * A helpler class for printing out a conic.
 */
class GeoConic {

  // $m is the GeoConic internal representation in terms of the
  // coefficients of the equation:
  //
  //   ax^2 + by^2 + c + dxy + ex + fy = 0
  //
  // See GeoGebra's documentation for GeoConicND::setCoeffs().
  var $m = array();
  var $zero_rhs = FALSE; // Whether to print "= 0" on the RHS, or to print the constant there.

  function __construct($m) {
    $m[3] *= 2;
    $m[4] *= 2;
    $m[5] *= 2;
    $this->m = $m;
  }

  function toString() {
    $s = '';

    if ($this->m[0]) {
      $s .= ' + ' . $this->m[0] . 'x^2';
    }
    if ($this->m[1]) {
      $s .= ' + ' . $this->m[1] . 'y^2';
    }
    if ($this->zero_rhs) {
      if ($this->m[2]) {
        $s .= ' + ' . $this->m[2];
      }
    }
    if ($this->m[3]) {
      $s .= ' + ' . $this->m[3] . 'x y';
    }
    if ($this->m[4]) {
      $s .= ' + ' . $this->m[4] . 'x';
    }
    if ($this->m[5]) {
      $s .= ' + ' . $this->m[5] . 'y';
    }

    if ($this->zero_rhs) {
      $s .= ' = 0';
    }
    else {
      $s .= ' = ' . (-1 * $this->m[2]);
    }

    $s = preg_replace('/^\s*\+\s*/', '', $s); // Remove leading "+".
    $s = preg_replace('/(-?)1(\s*[xyz])/', '\1\2', $s); // Remove "1" coefficients.
    $s = preg_replace('/\+ -/', '- ', $s); // Fix "+ -2x".

    return $s;
  }
}
