<?php

/**
 * A class to handle paths.
 */
class Paths {

  private $url;

  private $temp_seed = NULL;
  private $temp_list = array();

  function __construct($url) {
    $this->url = $url;
    $this->temp_seed = $_SERVER['REQUEST_TIME'] . '-' . rand();
  }

  function get_download_url() {
    return $this->url;
  }

  function output_base() {
    return md5($this->url);
  }

  /**
   * Returns path to an output file.
   */
  function output($ext) {
    return 'cache/' . $this->output_base() . '.' . $ext;
  }

  /**
   * Returns path to a temporary file.
   *
   * Note that repeated calls with the same $ext will return the same path.
   * That's a feature. So, if you want two XML files, do temp('one.xml') and temp('two.xml').
   */
  function temp($ext) {
    $filename = sprintf('tmp/tmp-%s.%s', $this->temp_seed, $ext);
    $this->temp_list[] = $filename;
    return $filename;
  }

  /**
   * Deletes all the temporary files generated in this run.
   */
  function delete_temps() {
    foreach ($this->temp_list as $file) {
      if (file_exists($file)) {
        unlink($file);
      }
    }
  }

  /**
   * Deletes stale output files.
   */
  function gc() {
    // We delete files older than 14 days.
    system('find cache/ -mtime +14 -exec rm "{}" \;');
    // If network fetch fails, we retry after 5 minutes.
    system(sprintf('find cache/ -mmin +5 -name "*.png" -size %dc -exec rm "{}" \;', filesize('message-no-access.png')));
  }
}

