<?php

/**
 * Unzips a file.
 *
 * @return int The shell exit code. Zero on success.
 */
function unzip($zip, $filename, $output) {
  exec(sprintf('unzip -p %s %s > %s', $zip, $filename, $output), $output, $exit_code);
  return $exit_code;
}

/**
 * Converts a relative path to an absolute URL.
 *
 * (We use it for the Location header.)
 */
function absolutize_url($path) {
  return 'http://' . $_SERVER['HTTP_HOST'] . dirname($_SERVER['SCRIPT_NAME']) . '/' . $path;
}

/**
 * Transfers a .ggb file from remote server into local temporary folder.
 *
 * if the URL has ";<number>" tucked at its end, it's considered to be an HTML
 * page with a Base64 applet embedded in it.
 *
 * An example for such applet is http://rangercollege.edu/math/ggb30/wind_triangle.html
 * (linked from http://rangercollege.edu/math/geogebra/geogebra_links.htm ).
 */
function download_worksheet($url, $destination) {

  if (preg_match('/(.*);(\d+)$/', $url, $m)) {

    //
    // Extract a Base64 encoded worksheet.
    //

    $url  = $m[1];
    $part = $m[2];

    $parts = preg_split('/<applet/i', file_get_contents($url));
    if (!empty($parts[$part + 1])) {
      $html = $parts[$part + 1];
    }
    else {
      $html = end($parts);
    }

    if (preg_match('/
                    name \s* = \s* ["\']ggbBase64[\'"] \s+
                    value \s* = \s* ["\'] (.*?) [\'"]
                   /sx', $html, $m)) {
      file_put_contents($destination, base64_decode($m[1]));
    }
  }
  else {

    // The link points to a normal .ggb file. Do a straight download.

    copy($url, $destination);
  }
}

// Testing code.
if (php_sapi_name() == 'cli') {
  error_reporting(E_ALL);
  download_worksheet('http://rangercollege.edu/math/ggb30/wind_triangle.html;0', 'here.zip');
}
