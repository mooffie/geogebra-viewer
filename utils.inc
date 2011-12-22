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
