<?php

function build_output_multiple($urls) {
  $i = 1;
  foreach ($urls as $url) {
    $paths = new Paths($url);
    print sprintf("Processing %s ... [%d/%d]\n", $url, $i++, count($urls));
    // Delete previous output in case we don't have permission to overwrite these files:
    foreach (glob($paths->output('*')) as $file) {
      unlink($file);
    }
    build_output($paths);
    $paths->delete_temps();
  }
}

function _get_cache_dir() {
  $dummy_paths = new Paths('dummy');
  return dirname($dummy_paths->output('dummy'));
}

function main_commandline() {
  global $argc, $argv;

  if ($argc == 1) {
    print "Syntax: thumb.php [ --redo | --clear | URL ... ]\n";
  }
  elseif ($argv[1] == '--redo') {
    // Re-process all the URLs I've ever seen.
    $cache_dir = _get_cache_dir();
    $urls = array_map('file_get_contents', glob($cache_dir . '/*.url'));
    if (!$urls) {
      print "No *.url files were found in $cache_dir/ . Nothing to do.\n";
    }
    else {
      build_output_multiple($urls);
    }
  }
  elseif ($argv[1] == '--clear') {
    // Clear the cache folder.
    $cache_dir = _get_cache_dir();
    foreach (glob($cache_dir . '/*') as $file) {
      unlink($file);
    }
  }
  else {
    // URL(s) are on the command line.
    array_shift($argv);
    build_output_multiple($argv);
  }

}
