<?php

abstract class Highlighter {
  abstract static public function run($s);
}

/**
 * A highlighter for the GeoGebra language.
 */
class Highlighter_GG extends Highlighter {

  static private $macro_names = array();

  static public function add_macro_name($name) {
    self::$macro_names[$name] = TRUE;
  }

  static public function run($s) {
    // @todo: optimize for cases where only commands are present.
    //return preg_replace('/(\w+)\[/', '<a href="http://wiki.geogebra.org/en/$1_Command">$1</a>[', htmlspecialchars($s, ENT_NOQUOTES));
    $res = '';

    foreach (preg_split('/( "[^"]*" | (?:\w+)\[ | \/\* .*? \*\/ | \/\/.* )/ux', $s, -1, PREG_SPLIT_DELIM_CAPTURE) as $part) {
      $first = substr($part, 0, 1);
      if ($first == '"') {
        $res .= '<span class="syn-string">' . preg_replace('/\\\\(?:\w+|.)/', '<span class="syn-latex">$0</span>', htmlspecialchars($part, ENT_NOQUOTES)) . '</span>';
      }
      elseif ($first == '/' && (substr($part, 1, 1) == '*' || substr($part, 1, 1) == '/')) {
        $res .= '<span class="syn-comment">' . htmlspecialchars($part) . '</span>';
      }
      elseif (substr($part, -1) == '[') {
        $cmd = substr($part, 0, -1);
        $res .= self::command_link($cmd) . '[';
      }
      else {
        $res .= htmlspecialchars($part);
      }
    }

    return $res;
  }

  static protected function command_link($cmd) {
    if (isset(self::$macro_names[$cmd])) {
      $extra_class = 'custom';
      $url = '#macro__' . $cmd;
      $target = '_self';
    }
    else {
      $extra_class = 'standard';
      $url = Commands::wiki($cmd);
      $target = '_blank';
    }
    return sprintf('<a href="%s" target="%s" class="syn-command %s">%s</a>', $url, $target, $extra_class, $cmd);
  }

}

/**
 * A highlighter for the JavaScript language.
 */
class Highlighter_JS extends Highlighter {

  static public function run($s) {
    $res = '';

    foreach (preg_split('/( "(?:[^\\\\]|\\\\.)*?" | \'(?:[^\\\\]|\\\\.)*?\' | \/\* .*? \*\/ | \/\/[^\n]* )/sx', $s, -1, PREG_SPLIT_DELIM_CAPTURE) as $part) {
      $first = substr($part, 0, 1);
      if ($first == '"' || $first == "'") {
        $res .= '<span class="syn-js-string">' . htmlspecialchars($part) . '</span>';
      }
      elseif ($first == '/' && (substr($part, 1, 1) == '*' || substr($part, 1, 1) == '/')) {
        $res .= '<span class="syn-comment">' . htmlspecialchars($part) . '</span>';
      }
      else {
        $html = htmlspecialchars($part);
        $html = preg_replace('/\b(?:break|case|catch|continue|default|delete|do|else|false|for|function|if|in|new|null|return|switch|this|throw|true|try|typeof|var|void|while|with)\b/', '<span class="syn-js-keyword-1">$0</span>', $html);
        $html = preg_replace('/\b(?:ggbApplet|evalCommand|setUndoPoint|deleteObject|setValue|setCoords|setColor|setVisible|setLabelVisible|setLabelStyle|setFixed|setTrace|renameObject|setLayer|setLayerVisible|setLineStyle|setLineThickness|setPointStyle|setPointSize|setFilling|getPNGBase64|writePNGtoFile|isIndependent|isMoveable|getBase64|setBase64|setAnimating|setAnimationSpeed|startAnimation|stopAnimation|isAnimationRunning|getXcoord|getYcoord|getValue|getColor|getVisible|getValueString|getDefinitionString|getCommandString|getObjectType|exists|isDefined|getObjectNumber|getObjectName|getLayer|getLineStyle|getLineThickness|getPointStyle|getPointSize|getFilling|setMode|openFile|reset|refreshViews|setCoordSystem|setAxesVisible|setGridVisible|registerAddListener|registerRemoveListener|registerUpdateListener|registerObjectUpdateListener|registerRenameListener|registerClearListener|evalXML|setXML|getXML|getXML|getAlgorithmXML)\b/', '<span class="syn-js-keyword-2">$0</span>', $html);
        $res .= $html;
      }
    }

    return $res;
  }

}

/**
 * A highlighter for the Python language.
 */
class Highlighter_Python extends Highlighter {

  static public function run($s) {
    $res = '';

    foreach (preg_split('/( """.*?""" | "(?:[^\\\\]|\\\\.)*?" | \'(?:[^\\\\]|\\\\.)*?\' | \#[^\n]* )/sx', $s, -1, PREG_SPLIT_DELIM_CAPTURE) as $part) {
      $first = substr($part, 0, 1);
      if ($first == '"' || $first == "'") {
        $res .= '<span class="syn-js-string">' . htmlspecialchars($part) . '</span>';
      }
      elseif ($first == '#') {
        $res .= '<span class="syn-comment">' . htmlspecialchars($part) . '</span>';
      }
      else {
        $html = htmlspecialchars($part);
        // We omit "class", as it clashes with the previous HTML.
        $html = preg_replace('/\b(?:and|assert|break|continue|def|del|elif|else|except|exec|finally|for|from|global|if|import|in|is|lambda|not|or|pass|print|raise|return|try|while|None|True|False)\b/', '<span class="syn-js-keyword-1">$0</span>', $html);
        $html = preg_replace('/\b(?:ggbApplet|evalCommand|setUndoPoint|deleteObject|setValue|setCoords|setColor|setVisible|setLabelVisible|setLabelStyle|setFixed|setTrace|renameObject|setLayer|setLayerVisible|setLineStyle|setLineThickness|setPointStyle|setPointSize|setFilling|getPNGBase64|writePNGtoFile|isIndependent|isMoveable|getBase64|setBase64|setAnimating|setAnimationSpeed|startAnimation|stopAnimation|isAnimationRunning|getXcoord|getYcoord|getValue|getColor|getVisible|getValueString|getDefinitionString|getCommandString|getObjectType|exists|isDefined|getObjectNumber|getObjectName|getLayer|getLineStyle|getLineThickness|getPointStyle|getPointSize|getFilling|setMode|openFile|reset|refreshViews|setCoordSystem|setAxesVisible|setGridVisible|registerAddListener|registerRemoveListener|registerUpdateListener|registerObjectUpdateListener|registerRenameListener|registerClearListener|evalXML|setXML|getXML|getXML|getAlgorithmXML)\b/', '<span class="syn-js-keyword-2">$0</span>', $html);
        $res .= $html;
      }
    }

    return $res;
  }

}
