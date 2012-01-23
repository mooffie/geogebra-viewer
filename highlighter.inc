<?php

abstract class Highlighter {
  abstract static public function run($s);
}

/**
 * A highlighter for the GeoGebra language.
 */
class Highlighter_GG extends Highlighter {

  static public function run($s) {
    // @todo: optimize for cases where only commands are present.
    //return preg_replace('/(\w+)\[/', '<a href="http://wiki.geogebra.org/en/$1_Command">$1</a>[', htmlspecialchars($s, ENT_NOQUOTES));
    $res = '';

    foreach (preg_split('/( "[^"]*" | (?:\w+)\[ | \/\* .*? \*\/ | \/\/.* )/x', $s, -1, PREG_SPLIT_DELIM_CAPTURE) as $part) {
      $first = substr($part, 0, 1);
      if ($first == '"') {
        $res .= '<span class="syn-string">' . preg_replace('/\\\\(?:\w+|.)/', '<span class="syn-latex">$0</span>', htmlspecialchars($part, ENT_NOQUOTES)) . '</span>';
      }
      elseif ($first == '/' && (substr($part, 1, 1) == '*' || substr($part, 1, 1) == '/')) {
        $res .= '<span class="syn-comment">' . htmlspecialchars($part) . '</span>';
      }
      elseif (substr($part, -1) == '[') {
        $cmd = substr($part, 0, -1);
        $res .= sprintf('<a href="http://wiki.geogebra.org/en/%s_Command" target="_blank" class="syn-command">%s</a>[', $cmd, $cmd);
      }
      else {
        $res .= htmlspecialchars($part);
      }
    }

    return $res;
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
        $html = preg_replace('/\b(?:ggbApplet|evalCommand|getValue|setValue)\b/', '<span class="syn-js-keyword-2">$0</span>', $html);
        $res .= $html;
      }
    }

    return $res;
  }

}
