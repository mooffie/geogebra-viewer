<?php

require_once './paths.inc';

class Commands {

  static $properties = array();

  /**
   * Loads the properties file.
   */
  static function load() {

    $url = 'https://raw.github.com/geogebra/geogebra/master/desktop/geogebra/properties/command.properties';
    $cache = Paths::_output('command.properties.js');

    if (file_exists($cache)) {
      self::$properties = (array)json_decode(file_get_contents($cache));
    }
    else {
      foreach (explode("\n", file_get_contents($url)) as $line) {
        list($internal, $english) = explode('=', $line);
        self::$properties[$internal] = $english;
      }
      file_put_contents($cache, json_encode(self::$properties));
    }
  }

  /**
   * Returns the English name of a command.
   *
   * Commands appear in the construction using internal name ("CurveCartesian").
   * Often we're interested in their English name instead ("Curve").
   */
  public static function english($internal) {
    if (!self::$properties) {
      self::load();
    }
    return isset(self::$properties[$internal]) ? self::$properties[$internal] : $internal;
  }

  /**
   * Returns the wiki page of a command.
   */
  public static function wiki($command) {
    return sprintf('http://wiki.geogebra.org/en/%s_Command', self::english($command));
  }

}
