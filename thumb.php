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

// -- start of hack to enable HTML till it's official --
/*if (!empty($_SERVER['HTTP_ACCEPT_LANGUAGE'])) {
  if (strpos($_SERVER['HTTP_ACCEPT_LANGUAGE'], 'yi') !== FALSE) {
    if ($output_type == 'txt') {
      $output_type = 'html';
    }
  }
}*/
if ($output_type == 'txt') {
  $output_type = 'html';
}
if ($output_type == 'plain') {
  $output_type = 'txt';
}
// -- end of hack --

if (empty($url)) { // Debug run.
  define('DBG', TRUE);
  $url = 'zz.ggb';
  //$url = 'http://www.geogebra.org/forum/download/file.php?id=12401';
}
else {
  define('DBG', FALSE);
}

////////////////////////////////////////////////////////////////////////////////

/**
 * Builds the thumnbail and protocol files.
 *
 * The files are written in the cached output folder.
 */
function build_output($paths) {
  require_once './ggxml.inc';

  // Transfer .ggb file from remote server into local temporary folder.
  list($success, $is_base64) = download_worksheet($paths->get_download_url(), $paths->temp('zip'));

  if (!$success) {
    // Might happen on 404, or if we need to login in order to access the file (e.g., in case of CMS).
    copy('message-no-access.png', $paths->output('png'));
  }
  else {
    if (!unzip($paths->temp('zip'), 'geogebra_thumbnail.png', $paths->output('png'))) {
      // Some worksheets don't have a thumbnail: GG 3.0 files, base64 encoded applets, and .ggt files.
      copy('message-no-thumbnail.png', $paths->output('png'));

      // Perhaps we downloaded a partial file? Save the .ggb for debugging purposes.
      // @todo: REMOVE.
      copy($paths->temp('zip'), $paths->output('ggb.debug'));
    }
    if ($is_base64) {
      // Make a copy of a base64 applet .ggb so we can have a download link.
      copy($paths->temp('zip'), $paths->output('ggb'));
    }
  }

  file_put_contents($paths->output('url'), $paths->get_download_url());

  //
  // Build the construction protocol.
  //
  $worksheet_files = array();
  if (unzip($paths->temp('zip'), 'geogebra_macro.xml', $paths->temp('macro.xml'))) {
    $worksheet_files['macros'] = $paths->temp('macro.xml');
  }
  if (unzip($paths->temp('zip'), 'geogebra.xml', $paths->temp('main.xml'))) {
    // Note: .ggt files have only macro.xml.
    $worksheet_files['main'] = $paths->temp('main.xml');
  }
  if (unzip($paths->temp('zip'), 'geogebra_javascript.js', $paths->temp('js'))) {
    $worksheet_files['js'] = $paths->temp('js');
  }
  $worksheet = new Worksheet($worksheet_files);
  file_put_contents($paths->output('txt'), PlainOutput::stringify($worksheet));
  file_put_contents($paths->output('html'), HtmlOutput::stringify($worksheet));
}

function output($paths, $output_type) {
  $url = absolutize_url($paths->output($output_type));
  if (DBG) {
    echo ('Location: ' . $url);
  }
  else {
    header('Location: ' . $url);
  }
}

function main() {
  global $url;
  global $output_type;

  $paths = new Paths($url);

  if (!DBG && file_exists($paths->output('png'))) {
    // A cached output exists.
    output($paths, $output_type);
  }
  else {
    build_output($paths);
    output($paths, $output_type);
    $paths->delete_temps();
    $paths->gc();
  }
}

if (php_sapi_name() != 'cli') {
  main();
}
else {
  require_once './thumb.commandline.inc';
  main_commandline();
}
