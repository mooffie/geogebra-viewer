<?php
/**
 * @file
 *
 * Code to convert GG's xml to a textual "source code" (construction protocol).
 *
 * Usage example:
 *
 *   echo stringify_construction_file('geogebra.xml');
 */

require_once './output.inc';

/**
 * Represents a line in a construction protocol (e.g., "c = Max[a, b]").
 *
 * It represents either a definition or an event handler.
 */
class GGLine {
  var $labels = array();
  var $definition = '';

  var $is_colon = FALSE;
  var $is_hidden = FALSE;
  var $is_auxiliary = FALSE;
  var $is_command = FALSE;

  // If set to 'click', 'update' etc., this line describes a script.
  var $event = NULL;
  var $event_languages = '';

  // The GGB type. Used when printing out a macro.
  var $type = NULL;

  /**
   * param $labels
   *    An array of labels. Or one label. Or NULL is to not print any labels.
   */
  function __construct($labels, $definition) {
    if (!$labels) {
      $labels = array();
    } elseif (is_string($labels)) {
      $labels = array($labels);
    }
    $this->labels = $labels;
    $this->definition = $definition;
  }

  function _pretty_definition() {
    // GeoText definitions may have newlines in them. For easier reading, we indent from 2'nd line.
    return preg_replace('/\n/', "\n  ", $this->definition);
  }

  function is_function_definition() {
    return ($this->is_colon && strpos($this->definition, $this->labels[0] . '(') === 0);
  }

  function toString() {
    return PlainOutput::stringify($this);
  }

  function write($output) {
    if ($this->event) {
      $title = sprintf('%s.on%s', $this->labels[0], ucwords($this->event)); // E.g., "button1.onClick"
      $output->addEvent($title, $this->definition, $this->event_language);
    }
    else {
      if (!$this->labels || $this->is_function_definition()) {
        $def = $this->_pretty_definition();
      }
      else {
        $def = join($this->labels, ', ') . ($this->is_colon ? ': ' : ' = ') . $this->_pretty_definition();
      }
      $output->addDefinition($def, $this->is_hidden || $this->is_auxiliary);
    }
  }
}

/**
 * Represents a construction. That is, a collection of GGLine objects.
 */
class GGConstruction {
  var $lines = array();

  // The file format number. Usually it's also the version of GeoGebra used.
  var $version = NULL;

  function __construct() {
  }

  function add($line) {
    $this->lines[] = $line;
  }

  /**
   * Finds a certain line.
   */
  function find_by_label($label) {
    foreach ($this->lines as $line) {
      // The join() check makes it possible to find "I, , , = Intersect[a, b]" as well.
      if ($line->labels && $line->labels[0] == $label && join($line->labels, '') == $label) {
        return $line;
      }
    }
    return NULL;
  }

  /**
   * Returns TRUE if a label is already mentioned in the construction.
   */
  function is_mentioned($label) {
    foreach ($this->lines as $line) {
      if (in_array($label, $line->labels)) {
        return TRUE;
      }
    }
    return FALSE;
  }

  function toString() {
    return PlainOutput::stringify($this);
  }

  function write($output) {
    foreach ($this->lines as $line) {
      $line->write($output);
    }
    if ($this->version) {
      // I'm removing the "Created with" string. Does GeoGebra 5.0 still show "4.0"?
      $output->addEmptyLine();
      $output->addDefinition("// File format: GeoGebra $this->version");
    }
  }
}

/**
 * Utility functions.
 */
class GGUtils {

  static function get_args($elt) {
    $args = array();
    // The if() is a guard against missing <input> or <output> elemenents.
    if ($elt) {
      $max = count($elt->attributes());
      for ($i = 0; $i < $max; $i++) {
        // We use if(), not if(!empty()), because the latter wouldn't catch "0".
        if ($elt['a' . $i]) {
          $args[] = (string)$elt['a' . $i];
        }
      }
    }
    return $args;
  }

  /**
   * Whether a string looks like a number
   */
  function is_num($s) {
    return preg_match('/^-?\d*\.?\d*$/', $s);
  }

  /**
   * Whether a string looks like an integer
   */
  function is_integral($s) {
    return preg_match('/^-?\d+$/', $s);
  }

}

/**
 * "Parses" XML into a construction object (GGConstruction).
 */
class GGParser {
  var $cons = NULL;

  // How far to round numbers?
  var $decimals = 2;

  function __construct($xml = NULL) {
    $this->cons = new GGConstruction;
    if ($xml) {
      $this->parse_xml($xml);
    }
  }

  /**
   * A utility function. If a string looks like a number, it rounds it.
   * Otherwise it's just cast'ed into a string.
   */
  function num_or_str($s) {
    $s = (string)$s;
    if (preg_match('/^-?\d*\.?\d*$/', $s)) {
      $s = round((float)$s, $this->decimals);
    }
    return $s;
  }

  function handle_expression($elt) {
    $line = new GGLine((string)$elt['label'], (string)$elt['exp']);
    $this->cons->add($line);
  }

  function handle_command($elt) {
    $labels = GGUtils::get_args($elt->output);
    $args = GGUtils::get_args($elt->input);
    $line = new GGLine($labels, (string)$elt['name'] . '[' . join($args, ', ') . ']');
    $line->is_command = TRUE;
    $this->cons->add($line);
  }

  /**
   * Creates a construction line ("a = ...") for a free object.
   */
  function handle_free_object($elt) {

    $line = NULL;
    $def = '...';

    $type = (string) $elt['type'];

    switch($type) {
      case 'numeric':
        // Personally, i'd've preferred not to round free numbers.
        $def = $this->num_or_str($elt->value['val']);
        break;
      case 'boolean':
        $def = $elt->value['val'];
        break;
      case 'image':
        $def = 'new image("' . preg_replace('/^[^\\\\\/]*./', '', $elt->file['name']) . '")';
        break;
      case 'button':
        $def = 'Button["' . $elt->caption['val'] . '"]';
        break;
      case 'point3d':
        $coords = array('x' => $this->num_or_str($elt->coords['x']), 'y' => $this->num_or_str($elt->coords['y']), 'z' => $this->num_or_str($elt->coords['z']));
        $def = '(' . join($coords, ', ') . ')';
        break;
      case 'point':
        $coords = array('x' => $this->num_or_str($elt->coords['x']), 'y' => $this->num_or_str($elt->coords['y']));
        $def = '(' . join($coords, ', ') . ')';
        break;
      case 'vector3d':
        $coords = array('x' => $this->num_or_str($elt->coords['x']), 'y' => $this->num_or_str($elt->coords['y']), 'z' => $this->num_or_str($elt->coords['z']));
        $def = '(' . join($coords, ', ') . ') /* vector */';
        break;
      case 'vector':
        $coords = array('x' => $this->num_or_str($elt->coords['x']), 'y' => $this->num_or_str($elt->coords['y']));
        $def = '(' . join($coords, ', ') . ') /* vector */';
        break;
      case 'conic':
        $def = 'new conic ' . $elt->matrix->asXML();
        break;
      case 'implicitpoly':
        if (!empty($elt->userinput['value'])) {
          $def = $elt->userinput['value'];
        }
        break;
      case 'segment':
        $def = '/* segment */'; // Probably a GG bug.
        break;
      case 'plane3d':
        $def = '/* plane3d */'; // http://www.geogebra.org/forum/viewtopic.php?f=53&t=22171
        break;
      case 'angle':
        $def = $this->num_or_str(  ((float)$elt->value['val']/M_PI * 180)  ) . '°';
        break;
      case 'line':
        $coords = array('x' => $this->num_or_str($elt->coords['x']), 'y' => $this->num_or_str($elt->coords['y']), 'z' => $this->num_or_str($elt->coords['z']));
        $out = '';
        if ($coords['x']) {
          $out .= $coords['x'] . 'x';
        }
        if ($coords['y']) {
          if ($coords['x']) {
            $out .= ' + ';
          }
          $out .= $coords['y'] . 'y';
        }
        $out = preg_replace('/(-?)1(\w)/', '\1\2', $out); // Remove "1" coefficients.
        $out .= ' = ';
        $out .= -1 * $coords['z'];
        $def = $out;
        break;
    }

    $line = new GGLine((string)$elt['label'], $def);
    $this->cons->add($line);

    return $line;
  }

  function add_script($elt, $event, $script, $language) {
    $line = new GGLine((string)$elt['label'], $script);
    $line->event = $event;
    $line->event_language = $language;
    $this->cons->add($line);
  }

  /**
   * Returns a string of the form "2 ... 8.5" for sliders.
   */
  function get_slider($elt) {
    if ($elt->slider) {
      $is_angle = ((string)$elt['type'] == 'angle');
      $min = $elt->slider['min'];
      $max = $elt->slider['max'];
      $step = '1';
      if ($is_angle) {
        // Convert to degrees.
        if (GGUtils::is_num($max)) {
          $max = $this->num_or_str(  ((float)$max/M_PI * 180)  ) . '°';
        }
        if (GGUtils::is_num($min)) {
          $min = $this->num_or_str(  ((float)$min/M_PI * 180)  ) . '°';
        }
      }
      else {
        if (!empty($elt->animation['step'])) {
          $step = $this->num_or_str($elt->animation['step']);
        }
        if (strpos($this->num_or_str($min) . $this->num_or_str($max) . $step, '.') !== FALSE) {
          // There's a decimal number somewhere. So show everything as float.
          if (GGUtils::is_integral($min)) {
            $min = $min . '.0';
          }
          if (GGUtils::is_integral($max)) {
            $max = $max . '.0';
          }
        }
        else {
          // All are integers. Convert '4.0' to '4'.
          $min = $this->num_or_str($min);
          $max = $this->num_or_str($max);
        }
      }
      // Lastly, We convert 360*° to 360°.
      //return preg_replace('/\*°/', '°', sprintf('%s ... %s', $this->num_or_str($min), $this->num_or_str($max)));
      return preg_replace('/\*°/', '°', sprintf('%s ... %s', $min, $max));
    }
  }

  function add_animation($elt) {
     // <slider min="0" max="60" absoluteScreenLocation="true" width="100.0" x="242.0" y="192.0" fixed="true" horizontal="true"/>
     // <animation step="1" speed="Vitesse" type="1" playing="true"/>
    $def = (string)$elt['label'] . '.animate';
    if ($this->get_slider($elt)) {
      $def .= ': ' . $this->get_slider($elt);
    }
    $this->cons->add(new GGLine(NULL, $def));
  }

  function add_slider($elt) {
    if (($def = $this->get_slider($elt))) {
      $line = new GGLine((string)$elt['label'] . '.slider', $def);
      $line->is_colon = TRUE;
      $this->cons->add($line);
    }
  }

  function handle_properties($elt) {
    $line = $this->cons->find_by_label((string)$elt['label']);

    if (!$this->cons->is_mentioned((string)$elt['label'])) {
      // If the object was not yet constructed, do it now.
      $line = $this->handle_free_object($elt);
    }

    // Add properties
    if ($line) {
      $type = (string) $elt['type'];
      $line->type = $type;

      if (in_array($type, array('function', 'functionNVar', 'implicitpoly', 'conic', 'line'))) {
        if (!$line->is_command) {
          $line->is_colon = TRUE;
        }
      }

      if (!empty($elt->show['object'])) {
        if ($elt->show['object'] == 'false') {
          $line->is_hidden = TRUE;
        }
      }

      if (!empty($elt->auxiliary['val'])) {
        if ($elt->auxiliary['val'] == 'true') {
          $line->is_auxiliary = TRUE;
        }
      }

      // Add animation or slider.
      if (!empty($elt->animation['playing']) && $elt->animation['playing'] == 'true') {
        $this->add_animation($elt);
      } else {
        if (!$line->is_hidden) {
          $this->add_slider($elt);
        }
      }

      // Add event handlers.
      foreach (array('ggbscript' => 'GeoGebra', 'javascript' => 'JavaScript', 'python' => 'Python') as $slot => $language) {
        if ($elt->$slot) {
          $slot = $elt->$slot;
          if ($slot['val']) {
            $this->add_script($elt, 'click', $slot['val'], $language);
          }
          if ($slot['onUpdate']) {
            $this->add_script($elt, 'update', $slot['onUpdate'], $language);
          }
        }
      }

      if (!empty($elt->condition['showObject'])) {
        $line = new GGLine(NULL, sprintf('SetConditionToShowObject[%s, %s]', $elt['label'], $elt->condition['showObject']));
        $this->cons->add($line);
      }

      if (!empty($elt->trace['val'])) {
        if ($elt->trace['val'] == 'true') {
          $this->cons->add(new GGLine(NULL, sprintf('%s.trace', $elt['label'])));
        }
      }
      if (!empty($elt->spreadsheetTrace['val'])) {
        if ($elt->spreadsheetTrace['val'] == 'true') {
          $this->cons->add(new GGLine(NULL, sprintf('%s.recordToSpreadsheet', $elt['label'])));
        }
      }
    }
  }

  /**
   * param @xml the element containing <construction> element.
   */
  function parse_xml($xml) {

    // Get the version numebr. We aren't using it, just displaying it.
    if (!empty($xml['format'])) {
      $version = (string) $xml['format'];
      if (!empty($xml['version'])) {
        $version .= ' (' . $xml['version'] . ')';
      }
      $this->cons->version = $version;
    }

    if (!empty($xml->kernel->decimals['val'])) {
      $this->decimals = (integer)$xml->kernel->decimals['val'];
    }

    foreach ($xml->construction->children() as $elt) {
      if ($elt->getName() == 'expression') {
        $this->handle_expression($elt);
      }
      if ($elt->getName() == 'command') {
        $this->handle_command($elt);
      }
      if ($elt->getName() == 'element') {
        $this->handle_properties($elt);
      }
    }
  }

  function get_construction() {
    return $this->cons;
  }

}

/**
 * Returns a new GGParser object.
 *
 * For some reason, PHP doesn't let us do "return (new GGParser(...))->whatever", but
 * it does let us do "return some_func()->whatever".
 */
function _GGParser($xml = NULL) {
  $parser = new GGParser($xml);
  return $parser;
}


/**
 * A macro is basically a wrapper around a construction.
 */
class Macro {

  var $name;
  var $inputs;
  var $outputs;
  var $help;
  var $cons;

  function __construct($name, $inputs, $outputs, $help, $cons) {
    $this->name = $name;
    $this->inputs = $inputs;
    $this->outputs = $outputs;
    $this->help = $help;
    $this->cons = $cons;
  }

  /**
   * Writes out the construction as a macro. Meaning, input and output variable are noted.
   *
   * A syntax similar to a Pascal function is used.
   */
  function write($output) {

    // Input args.
    $args = array();
    foreach ($this->inputs as $arg) {
      if (($line = $this->cons->find_by_label($arg))) {
        $args[] = sprintf('%s: %s', $arg, $line->type);
      }
      else {
        $args[] = $arg;
      }
    }

    // Output args.
    $return_types = array();
    foreach ($this->outputs as $arg) {
      if (($line = $this->cons->find_by_label($arg))) {
        $return_types[] = $line->type;
      }
    }

    $signature = sprintf("macro %s(%s) as %s", $this->name, join(', ', $args), join(', ', $return_types));
    $output->beginMacro($this->name, $signature, $this->help);

    foreach ($this->cons->lines as $line) {
      if (@in_array($line->labels[0], $this->inputs)) {
        continue;
      }
      $line->write($output);
    }

    $output->addDefinition(sprintf("return %s", join(', ', $this->outputs)));
    $output->endMacro();
  }
}

/**
 * Represents a worksheet (a .ggb or .ggt file).
 */
class Worksheet {

  // The main construction.
  var $main = NULL;
  // Macros.
  var $macros = array();
  // JavaScript.
  var $js = '';

  function __construct($files) {
    $this->load($files);
  }

  function load($files) {
    if (!empty($files['main'])) {
      $xml = simplexml_load_file($files['main']);
      $this->main = _GGParser($xml)->get_construction();
    }
    if (!empty($files['macros'])) {
      $xml = simplexml_load_file($files['macros']);
      $this->parse_macros($xml);
    }
    if (!empty($files['js'])) {
      $js = trim(file_get_contents($files['js']));
      if (substr_count($js, "\n") > 0) {
        $this->js = $js;
      }
    }
  }

  function parse_macros($xml) {
    foreach ($xml->macro as $macro) {
      $this->macros[] = new Macro(
        (string)$macro['cmdName'],
        GGUtils::get_args($macro->macroInput),
        GGUtils::get_args($macro->macroOutput),
        (string)$macro['toolHelp'],
        _GGParser($macro)->get_construction()
      );
    }
  }

  function toString() {
    return PlainOutput::stringify($this);
  }

  function write($output) {
    foreach ($this->macros as $macro) {
      $macro->write($output);
      $output->addEmptyLine();
    }
    if ($this->main) {
      $this->main->write($output);
    }
    if ($this->js) {
      $output->addEmptyLine();
      $output->addDefinition('/* -------- Global JavaScript: -------- */');
      $output->addEmptyLine();
      $output->addScript($this->js, 'JavaScript');
      $output->addEmptyLine();
    }
  }
}


// Testing code.
if (0 && php_sapi_name() == 'cli') {
  error_reporting(E_ALL);

  function test1() {
    $files = array(
      'main' => 'geogebra.xml',
      'macros' => 'geogebra_macro.xml',
      'js' => 'geogebra_javascript.js',
    );
    $ws = new Worksheet($files);
    //$out = new PlainOutput();
    $out = new HtmlOutput();
    $ws->write($out);
    print $out->toString() . "\n";
    file_put_contents('out.html', $out->toString());
  }

  test1();
}
