<?php

function build_output_multiple($urls) {
  $i = 1;
  foreach ($urls as $url) {
    $paths = new Paths($url);
    print sprintf("Processing %s ... [%d/%d]\n", $url, $i++, count($urls));
    // Delete previous output in case we don't have permission to overwrite these files:
    foreach (glob(str_replace('.dummy', '.*', $paths->output('dummy'))) as $file) {
      unlink($file);
    }
    build_output($paths);
  }
}

function main_commandline() {
  global $argc, $argv;

  if ($argc == 1) {
    print "Syntax: thumb.php [ --all | URL ... ]\n";
  }
  elseif ($argv[1] == '--all') {
    // Re-process all the URLs I've even seen.
    $dummy_paths = new Paths('dummy');
    $cache_dir = dirname($dummy_paths->output('dummy'));
    $url_files = glob($cache_dir . '/*.url');
    if (!$url_files) {
      print "No *.url files were found in $cache_dir/ . Nothing to do.\n";
    }
    else {
      $urls = array();
      foreach ($url_files as $fname) {
        $urls[] = file_get_contents($fname);
      }
      build_output_multiple($urls);
    }
  }
  else {
    // URL(s) are on the command line.
    array_shift($argv);
    build_output_multiple($argv);
  }

}
