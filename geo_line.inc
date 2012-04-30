<?php

/**
 * A helpler class for printing out a line.
 */
class GeoLine {

  // The coefficients of ax + by + c = 0.
  var $x, $y, $z;

  // For rounding.
  var $decimals;

  function __construct($x, $y, $z, $decimals) {
    // We convert any 0.0 to 0 because in PHP -1*0.0 gives -0, and we don't want this.
    $this->x = $x ? $x : 0;
    $this->y = $y ? $y : 0;
    $this->z = $z ? $z : 0;
    $this->decimals = $decimals;
  }

  function toString() {
    list($x, $y, $z) = array($this->x, $this->y, $this->z);

    $s = '';

    if ($x && $y) {
      // Format as "y = mx + b".
      if ($y < 0) {
        $x *= -1;
        $y *= -1;
        $z *= -1;
      }
      // Move x and z to the RHS.
      $x *= -1;
      $z *= -1;
      // Build m and b.
      $m = round($x / $y, $this->decimals);
      $b = round($z / $y, $this->decimals);
      $s = "y = ${m}x";
      if ($b) {
        $s .= ' + ' . $b;
      }
    }
    else {
      // Format as "ax + by = c".
      if ($x) {
        $s .= $x . 'x';
      }
      if ($y) {
        if ($x) {
          $s .= ' + ';
        }
        $s .= $y . 'y';
      }
      $s .= ' = ';
      $s .= -1 * $z;

    }

    $s = preg_replace('/(-?)1(\w)/', '\1\2', $s); // Remove "1" coefficients.
    $s = preg_replace('/\+ -/', '- ', $s); // Fix "+ -2whatever".

    return $s;
  }
}
