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

/**
 * Represents a line in a construction protocol (e.g., "c = Max[a, b]").
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

  function pretty_definition() {
    // GeoText definitions may have newlines in them. For easier reading, we indent from 2'nd line.
    return preg_replace('/\n/', "\n  ", $this->definition);
  }

  function is_function_definition() {
    return ($this->is_colon && strpos($this->definition, $this->labels[0] . '(') === 0);
  }

  function _toString() {
    if ($this->event) {
      return sprintf("%s.on%s = {\n\n    %s\n\n}", $this->labels[0], ucwords($this->event), join(preg_split('/\n/', trim($this->definition)), "\n    "));
    }
    else {
      if (!$this->labels || $this->is_function_definition()) {
        return $this->pretty_definition();
      }
      else {
        return join($this->labels, ', ') . ($this->is_colon ? ': ' : ' = ') . $this->pretty_definition();
      }
    }
  }

  function toString() {
    $s = $this->_toString();
    if ($this->is_hidden || $this->is_auxiliary) {
      return "($s)";
    }
    else {
      return $s;
    }
  }

  // Debugging aid.
  function prt() {
    echo $this->toString() . "\n";
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
      if (count($line->labels) == 1 && $line->labels[0] == $label) {
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
    $out = '';
    foreach ($this->lines as $line) {
      $out .= $line->toString() . "\n";
    }
    if ($this->version) {
      // I'm removing the "Created with" string. Does GeoGebra 5.0 still show "4.0"?
      $out .= "\n// File format: GeoGebra $this->version\n";
    }
    return $out;
  }

  /**
   * Writes out the construction as a macro. Meaning, input and output variable are noted.
   *
   * A syntax similar to a Pascal function is used.
   */
  function toString_as_macro($name, $inputs, $outputs, $help) {
    $out = '';

    // Input args.
	$args = array();
	foreach ($inputs as $arg) {
	  if (($line = $this->find_by_label($arg))) {
	    $args[] = sprintf('%s: %s', $arg, $line->type);
	  }
	  else {
	    $args[] = $arg;
	  }
	}

    // Output args.
	$return_types = array();
	foreach ($outputs as $arg) {
	  if (($line = $this->find_by_label($arg))) {
	    $return_types[] = $line->type;
	  }
	}

    if ($help) {
      $out .= "/* $help */\n";
    }
    $out .= sprintf("macro %s(%s) as %s\n", $name, join(', ', $args), join(', ', $return_types));
    foreach ($this->lines as $line) {
      if (@in_array($line->labels[0], $inputs)) {
        continue;
      }
      $out .= '  ' . $line->toString() . "\n";
    }
    $out .= sprintf("  return %s\n", join(', ', $outputs));
    $out .= "end";
    return $out;
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
        $def = 'new image("' . preg_replace('/^[^\\\\]*./', '', $elt->file['name']) . '")';
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

  function add_script($elt, $event, $script) {
    $line = new GGLine((string)$elt['label'], $script);
    $line->event = $event;
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
    $def = '';
    if ($this->get_slider($elt)) {
      $def = $this->get_slider($elt);
    }
    $line = new GGLine((string)$elt['label'] . '.animate', $def);
    $line->is_colon = TRUE;
    $this->cons->add($line);
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

      if (!empty($elt->ggbscript['val'])) {
        $this->add_script($elt, 'click', $elt->ggbscript['val']);
      }
      if (!empty($elt->ggbscript['onUpdate'])) {
        $this->add_script($elt, 'update', $elt->ggbscript['onUpdate']);
      }
      if (!empty($elt->javascript['val'])) {
        $this->add_script($elt, 'click', $elt->javascript['val']);
      }
      if (!empty($elt->javascript['onUpdate'])) {
        $this->add_script($elt, 'update', $elt->javascript['onUpdate']);
      }

      if (!empty($elt->condition['showObject'])) {
        $line = new GGLine(NULL, sprintf('SetConditionToShowObject[%s, %s]', $elt['label'], $elt->condition['showObject']));
        $this->cons->add($line);
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

function stringify_construction_xml($xml) {
  return _GGParser($xml)->get_construction()->toString();
}

function stringify_construction_file($file) {
  return stringify_construction_xml(simplexml_load_file($file));
}

function stringify_macro_xml($xml) {
  $out = '';
  foreach ($xml->macro as $macro) {
    $help = (string)$macro['toolHelp'];
    $inputs = GGUtils::get_args($macro->macroInput);
    $outputs = GGUtils::get_args($macro->macroOutput);
    $cons = _GGParser($macro)->get_construction();
    $out .= $cons->toString_as_macro($macro['cmdName'], $inputs, $outputs, $help) . "\n\n";
  }
  return $out;
}

function stringify_macro_file($file) {
  return stringify_macro_xml(simplexml_load_file($file));
}

// Testing code.
if (php_sapi_name() == 'cli') {
  error_reporting(E_ALL);
  echo stringify_construction_file('geogebra.xml');
  exit();
  //echo stringify_macro_file('geogebra_macro.xml');
}
