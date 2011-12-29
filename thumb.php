<?php
/**
 * @file
 *
 * The main entry point to the thumbnailing application.
 */

error_reporting(E_ALL);

require_once './paths.inc';
require_once './utils.inc';

////////////////////////////////////////////////////////////////////////////////

$url         = empty($_GET['url'])         ? ''    : $_GET['url'];
$output_type = empty($_GET['output_type']) ? 'png' : $_GET['output_type'];

if (empty($url)) { // Debug run.
  define('DBG', TRUE);
  $url = 'zz.ggb';
  //$url = 'http://www.geogebra.org/forum/download/file.php?id=12401';
}
else {
  define('DBG', FALSE);
}

$paths = new Paths($url);

////////////////////////////////////////////////////////////////////////////////

/**
 * Builds the thumnbail and protocol files.
 *
 * The files are written in the cached output folder.
 */
function build_output() {
  global $paths;
  require_once './ggxml.inc';

  // Transfer .ggb file from remote server into local temporary folder.
  download_worksheet($paths->get_download_url(), $paths->temp('zip'));

  if (unzip($paths->temp('zip'), 'geogebra_thumbnail.png', $paths->output('png')) != 0) {
    // .ggt files don't have a thumbnail.
    copy('1x1.png', $paths->output('png'));
  }
  file_put_contents($paths->output('url'), $paths->get_download_url());

  //
  // Build the construction protocol.
  //
  $protocol = '';
  if (unzip($paths->temp('zip'), 'geogebra_macro.xml', $paths->temp('macro.xml')) == 0) {
    $protocol .= stringify_macro_file($paths->temp('macro.xml'));
  }
  if (unzip($paths->temp('zip'), 'geogebra.xml', $paths->temp('main.xml')) == 0) {
    // Note: .ggt files have only macro.xml.
    $protocol .= stringify_construction_file($paths->temp('main.xml'));
  }
  if (unzip($paths->temp('zip'), 'geogebra_javascript.js', $paths->temp('js')) == 0) {
    $js = file_get_contents($paths->temp('js'));
    if (substr_count(trim($js), "\n") > 0) {
      $protocol .= "\n/* -------- Global JavaScript: -------- */\n\n$js\n";
    }
  }
  file_put_contents($paths->output('txt'), $protocol);
}

function output($output_type) {
  global $paths;

  $url = absolutize_url($paths->output($output_type));
  if (DBG) {
    echo ('Location: ' . $url);
  }
  else {
    header('Location: ' . $url);
  }
}

function main() {
  global $paths;
  global $output_type;

  if (!DBG && file_exists($paths->output('png'))) {
    // A cached output exists.
    output($output_type);
  }
  else {
    build_output();
    output($output_type);
    $paths->delete_temps();
    $paths->gc();
  }
}

main();
