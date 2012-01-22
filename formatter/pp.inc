<?php

error_reporting(E_ALL);

mb_internal_encoding('UTF-8');

// TODO:
//   - cleanup.
//   - Ignore 'Element[]' and '{}' inside strings...
//   - doShift: if we're on the first line (a, b, c, d, ,, = Polygon[]), limit to, say, 20.
//
//   - add trailing space to the thumbnails output so it's not easy to skip copying the last ']'.

class Scanner {

  // The string we're processing.
  var $s = '';

  // Current position inside the string.
  //
  // Note: we treat the string as binary. That is, $pos is byte-based
  // (not char-based). That's because we're examining ASCII chars
  // only ({}, [], ") and we want to conserve CPU cycles. One consequence
  // is that we'll be using substr/strlen etc instead of mb_substr/mb_length.
  var $pos = 0;

  function __construct($s) {
    $this->s = $s;
  }

  function show() {
    printf("<%s>\n", substr($this->s, $this->pos));
  }

  function setPos($pos) {
    $this->pos = $pos;
  }

  function getPos() {
    return $this->pos;
  }

  function atEOS() {
    return $this->pos >= strlen($this->s);
  }

  /**
   * Returns the next character.
   *
   * Returns FALSE if at EOS, or past EOS.
   */
  function readChar() {
    $ch = substr($this->s, $this->pos, 1);
    if ($ch !== FALSE) {
      $this->pos++;
    }
    return $ch;
  }

  function moveToEOS() {
    $this->pos = strlen($this->s) + 1;
  }

  function readTail() {
    $ret = substr($this->s, $this->pos);
    $this->moveToEOS();
    return $ret;
  }

  /**
   * Returns the string from $pos to the newline character (including).
   */
  function readLine() {
    $line = '';
    for (;;) {
      $ch = $this->readChar();
      if ($ch === FALSE) {
        return $line;
      }
      $line .= $ch;
      if ($ch == "\n" || $ch == "\r") {
        return $line;
      }
    }
  }

  /**
   * Like readLine() but does not advance $pos;
   */
  function peekLine() {
    $line = $this->readLine();
    $this->pos -= strlen($line);
    return $line;
  }

  /**
   * Read till the next "," (of till end of string).
   *
   * Commas inside brackes/parenthesis or strings are ignored.
   */
  function readExpr() {
    $level = 0;
    $s = $this->s;
    $chunk = '';
    $in_string = FALSE;

    for (;;) {
      $ch = substr($s, $this->pos, 1);
      if ($ch === FALSE) {
        // End of string.
        break;
      }

      if ($ch == '"') {
        $in_string = !$in_string;
      }

      if (!$in_string) {
        if ($ch == '[' || $ch == '(' || $ch == '{') {
          $level++;
        }
        if ($ch == ']' || $ch == ')' || $ch == '}') {
          $level--;
        }
        if ($ch == ',' && $level == 0) {
          break;
        }
      }

      $chunk .= $ch;
      $this->pos++;
    }

    //print '-'.$chunk."-\n";
    return $chunk;
  }

  /**
   * Reads a bracketed/parenthesized expression.
   *
   * @todo: should we return $success if the expr is really balanced
   * (and not terminated just b/c we reached end of string)? No, I think we
   * can ignore that.
   */
  function readBracketsExpr() {
    $level = 0;
    $s = $this->s;
    $chunk = '';
    $in_string = FALSE;

    $stop = FALSE;
    for ( ; !$stop; ) {
      $ch = substr($s, $this->pos, 1);
      if ($ch === FALSE) {
        // End of string.
        break;
      }

      if ($ch == '"') {
        $in_string = !$in_string;
      }

      if (!$in_string) {
        if ($ch == '[' || $ch == '(' || $ch == '{') {
          $level++;
        }
        if ($ch == ']' || $ch == ')' || $ch == '}') {
          $level--;
          if ($level == 0) {
            $stop = TRUE;
          }
        }
      }

      $chunk .= $ch;
      $this->pos++;
    }

    //print '-'.$chunk."-\n";
    return $chunk;
  }

  /**
   * Splits the string into top-level commands, lists, and "everything else".
   */
  function split() {
    $parts = array();

    $this->setPos(0);

    while (preg_match('/ [\w\d]+\[ | \{ /ux', $this->s, $m, PREG_OFFSET_CAPTURE, $this->getPos())) {
      $where = $m[0][1];
      // @todo: Warning: is it safe to assume $where is a binary offest?

      if ($where != $this->getPos()) {
        $parts[] = new S_Bin(substr($this->s, $this->getPos(), $where - $this->getPos()));
      }

      $this->setPos($where);
      //$this->show();
      //$expr = $this->readBracketsExpr();
      //$parts[] = (substr($expr, 0, 1) == '{') ? (new S_List($expr)) : (new S_Command($expr));
      $parts[] = S_Part::factory($this->readBracketsExpr());
    }

    if (!$this->atEOS()) {
      $parts[] = new S_Bin($this->readTail());
    }

    return $parts;
  }

}

class Formatter {

  // If doShift is TRUE, commands/lists are printed as...
  //
  // 3 + Cmd[
  //     ]
  //
  // ...instead of:
  //
  // 3 + Cmd[
  // ]
  //
  // (Since I don't see why anybody would NOT want this, I may remove this
  // switch in the future.)
  static $doShift = TRUE;

  /**
   * "Pretty prints" a string.
   *
   * param @is_top_level
   *   Optional. Boolean. Whether to indent 2'nd line onward to the length of "var = ".
   */
  static function pp($scanner, $is_top_level = FALSE) {
    // @tood: optimize: quit if doens't have [ {

    $parts = $scanner->split();
    $s = '';
    foreach ($parts as $part) {
      $pp = $part->pp();
      // For explanation, see documentation for $doShift.
      if (Formatter::$doShift && strpos($pp, "\n") !== FALSE) {
        $pp = S_Utils::indent($pp, str_repeat(' ', mb_strlen(S_Utils::lastLine($s))), TRUE);
      }
      $s .= $pp;
    }

/*
    // ** The following isn't needed anymore, as $doShift already does this. **

    if ($is_top_level) {
      // Top level commands usually start with "a = " (possibly with more than
      // one variable), so we indent to this level.
      if (preg_match('/^ \s* \(? [\w\d]+ ( \s* , \s* ([\w\d]+)? )* \s* [:=] \s* /ux', $s, $m)) {
        $width = mb_strlen($m[0]);
        if ($width > 20) {
          $width = 20;
        }
        $s = S_Utils::indent($s, str_repeat(' ', $width), TRUE);
      }
    }
*/
    return $s;
  }

}

class S_Part {

  var $s;

  function __construct($s) {
    $this->s = $s;
  }

  /**
   * "Pretty prints" this string.
   */
  function pp() {
    return $this->s;
  }

  static function factory($s) {
    if (preg_match('/^[\w\d]+\[.*\]$/u', $s)) {
      return new S_Command($s);
    }
    if (preg_match('/^{.*}$/', $s)) {
      return new S_List($s);
    }
    return new S_Bin($s);
  }
}


class S_Command extends S_Part {

  var $name = '';
  var $raw_args = '';
  var $args = NULL;

  // How much to indent arguments. Two spaces seem ok.
  static $indent_string = '  ';

  function __construct($s) {
    parent::__construct($s);
    $this->init($s);
  }

  function init($s) {
    $idx = strpos($s, '[');
    $this->name = substr($s, 0, $idx);
    $this->raw_args = substr($s, $idx + 1, -1);
  }

  /**
   *
   */
  function initArgs() {
    if (!isset($this->args)) {
      $this->args = array();
      $scn = new Scanner($this->raw_args);
      while (!$scn->atEOS()) {
        $this->args[] = trim($scn->readExpr());
        // Eat up the comma in between:
        $scn->readChar();
      }
    }
  }

  /**
   * Returns TRUE is this is a "simple" command. That is, if we shouldn't split it into several lines.
   */
  function isSimple() {
    return (strpos($this->raw_args, '[') === FALSE && strpos($this->raw_args, '{') === FALSE && strlen($this->raw_args) < 50);
  }

  /**
   * Lumps trailing "simple" arguments together.
   *
   * This ensures that "Sequence[whatever, n, 1, 10]" is printed as:
   *
   *   Sequence[
   *     whatever,
   *     n, 1, 10
   *   ]
   */
  function compactArgs() {
    $simples = array();
    while ($this->args && S_Utils::isVerySimple(end($this->args))) {
      $simples[] = array_pop($this->args);
    }
    if ($simples) {
      $this->args[] = join(', ', array_reverse($simples));
    }
  }

  function envelope($s) {
    return $this->name . '[' . $s . ']';
  }

  function pp() {
    if ($this->isSimple()) {
      return $this->ppSimple();
    }
    else {
      return $this->ppComplex();
    }
  }

  function ppSimple() {
    return $this->envelope($this->raw_args);
  }

  function ppComplex() {
    $this->initArgs();
    $this->compactArgs();

    $args_pp = array();
    foreach ($this->args as $arg) {
      //$scn = new Scanner($arg);
      //$args_pp[] = $scn->pp();
      $args_pp[] = Formatter::pp(new Scanner($arg));
    }

    $s = S_Utils::indent(join(",\n", $args_pp), S_Command::$indent_string) . "\n";

    //return "$this->name[<" . join('>, <', $this->args) . ">]";
    return $this->envelope("\n" . $s);
  }

}

class S_List extends S_Command {

  function init($s) {
    $this->name = 'List'; // Dummy; not really used.
    $this->raw_args = substr($s, 1, -1);
  }

  function envelope($s) {
    return '{' . $s . '}';
  }

  /**
   * Returns TRUE is this is a "simple" command. That is, if we shouldn't split it into several lines.
   */
  function isSimple() {

    if (strpos($this->raw_args, '[') !== FALSE || strpos($this->raw_args, '{') !== FALSE) {
      return FALSE;
    }

    // Examine strings that don't contain commands/lists:

    if (strlen($this->raw_args) > 150) {
      // Giant lists are "simple": we don't want to waste CPU cycles.
      return TRUE;
    }

    if (strlen($this->raw_args) > 50) {
      if (!preg_match('/[^0-9. (,)-]/', $this->raw_args)) {
        // If a long list contains only pure numbers/points, it's "simple",
        // because we don't want to waste CPU cycles.
        return TRUE;
      }
      else {
        return FALSE;
      }
    }

    return TRUE;
  }

}

/**
 * Represents anything which isn't a command or list; that is, anything in
 * between and inside. We treat it as "binary" and print it as-is.
 */
class S_Bin extends S_Part {
}


/**
 * Utilily functions.
 */
class S_Utils {

  /**
   * Indents all lines in a string.
   *
   * @param $skip_first
   *   Whether to skip indenting the first line.
   */
  public static function indent($s, $space, $skip_first = FALSE) {
    return ($skip_first ? '' : $space) . str_replace("\n", "\n" . $space, $s);
  }

  /**
   * Detects "very simple" expressions.
   *
   * These are of the form "12.2", "n", "n + 1", "2 * n + k", but not longer.
   */
  public static function isVerySimple($s) {
    return preg_match('/^ [\w\d.-]+ ( \s [+-\/*] \s [\w\d.-]+ ){0,2} $/ux',  $s);
  }

  /**
   * Returns the last line of a chunk of text.
   */
  public static function lastLine($s) {
    $lines = explode("\n", $s);
    return end($lines);
  }

  /**
   * "Pretty prints" definitions.
   */
  public static function pp($s) {
    $lines = preg_split('/(\r?\n|\r)/', $s);
    $out = '';
    foreach ($lines as $line) {
      $out .= Formatter::pp(new Scanner($line), TRUE) . "\n";
    }
    return $out;
  }

  /**
   * Does the opposite of pp().
   */
  public static function unPp($s) {
    // Canonize input: make sure only a single "\n" separates lines.
    // splitDefinitions($s)
    $s = preg_replace('/\r?\n/', "\n", $s);
    // I'm not sure the following is needed. For rogue Mac browsers?
    $s = preg_replace('/\r/', "\n", $s);

    $defs = S_Utils::splitDefinitions($s);
    $text = '';
    foreach ($defs as $def) {
      $def = preg_replace('/\n\s*/', ' ', $def);
      $def = preg_replace('/([{[]) /', '$1', $def);
      $def = preg_replace('/ ([]}])/', '$1', $def);
      $def = trim($def);
      $text .= $def . "\n";
    }
    return $text;
  }

  /**
   * Splits a "pretty" text into definitions.
   */
  static function splitDefinitions($s) {
    $scn = new Scanner($s);
    $defs = array();

    while (!$scn->atEOS()) {
      // Read the next definition.
      $def = '';
      for(;;) {
        $rest_of_line = $scn->peekLine();
        if (preg_match('/[{([]/', $rest_of_line)) {
          $def .= $scn->readBracketsExpr();
        }
        else {
          $def .= $scn->readLine();
          $defs[] = $def;
          break;
        }
      }
    }

    return $defs;
  }
}
