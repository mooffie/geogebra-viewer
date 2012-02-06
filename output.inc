<?php
/**
 * @file
 *
 * Contain 'Output' classes, which stringify a construction.
 */

abstract class Output {
  abstract function addDefinition($def, $hidden = false);
  abstract function addEmptyLine();
  abstract function addScript($block, $language);
  abstract function addEvent($event, $block, $language);
  abstract function beginMacro($name, $signature, $help);
  abstract function endMacro();
}

/**
 * Writes the output as plain text.
 */
class PlainOutput extends Output {

  private $out = '';
  private $in_macro = FALSE;

  function __construct() {
  }

  function toString() {
    return $this->out;
  }

  function addDefinition($def, $hidden = false) {
    if ($hidden) {
      $def = '(' . $def . ')';
    }
    if ($this->in_macro) {
      $def = '  ' . $def;
    }
    $this->out .= $def . "\n";
  }

  function addEmptyLine() {
    $this->addDefinition('');
  }

  function addScript($block, $language) {
    $this->out .= $block;
  }

  function addEvent($event, $block, $language) {
    $this->out .= sprintf("%s = {\n\n    %s\n\n}\n", $event, join(explode("\n", trim($block)), "\n    "));
  }

  function beginMacro($name, $signature, $help) {
    if ($help) {
      $this->addDefinition("/* $help */");
    }
    $this->addDefinition($signature);
    $this->in_macro = TRUE;
  }

  function endMacro() {
    $this->in_macro = FALSE;
    $this->addDefinition('end');
  }

  static public function stringify($obj) {
    $output = new self();
    $obj->write($output);
    return $output->toString();
  }
}

//////////////////////////////// HTML: ////////////////////////////////////////

require_once './formatter/pp.inc';
require_once './highlighter.inc';

/**
 * Writes the output as HTML.
 *
 * Most notably, it allows to "expand" a line of GeoGebra code into a
 * pretty-printed one. This expand/collapse feature could have been implemented
 * very easily using CSS almost exclusively. However, IE copies to clipboard
 * text that is "display: none" so we have to use JavaScript to shift in/out
 * the expanded/collapsed versions.
 */
class HtmlOutput extends Output {

  private $out = '';
  private $counter = 1;
  private $indent = 0;
  private $initial_state = 'collapsed'; // You may change to 'expanded'.
  private $js_states = array();

  // Ideally, to conserve space, we wouldn't have to store in $js_states
  // the state that's already in the HTML. However, some browsers (IE) are
  // buggy and don't let us read the HTML with high-fidelity: newlines/spaces
  // are squashed. So we have store in $js_states both states.
  private $send_both_states = FALSE;

  protected function addIndent() {
    $this->indent++;
  }

  protected function removeIndent() {
    $this->indent--;
  }

  function __construct() {
  }

  function toString() {
    list($prolog, $epilog) = explode('{{OUTPUT}}', file_get_contents('output.template.html'));
    $prolog = strtr($prolog, array(
      '{{STATES}}' => json_encode($this->js_states),
      '{{INITIAL-STATE}}' => $this->js_states ? $this->initial_state : '',
    ));
    return $prolog . $this->out . $epilog;
  }

  /**
   * param $text
   *   The text to add. It must be HTML-ready (e.g., after htmlescape()).
   */
  protected function addRow($text, $extra_classes = '', $include_button = FALSE, $return_only = FALSE) {

    $classes = 'row';
    $classes .= ' row-' . $this->counter;
    $classes .= ' ' . ($this->counter%2 == 0 ? 'even' : 'odd');
    if ($extra_classes) {
      $classes .= ' ' . $extra_classes;
    }

    $button = "<span class='button-wrapper'><span class='button'></span></span>";

    // We implement indentation using spaces instead of CSS because we want
    // copy/paste to preserve this indentation.
    $space = str_repeat(' ', $this->indent * 2);
    if ($space) {
      $text = str_replace("\n", "\n" . $space, $text);
    }

    $text = $space . ($include_button ? $button : '') . $text;

    $out = "<div class='$classes'>" . $text . "\n</div>";

    // Sheesh! It turns out some browsers can't keep spaces (when copying to
    // clipboard, at least) inside a <pre><div></div></pre> (even when we use
    // style 'white-space: pre' on the div), so we have to escape it for them.
    // (Culprits: IE, Firefox; Sages: Opera, Chrome.)
    $out = preg_replace_callback('/  +/', create_function('$m', 'return str_repeat("\\xC2\\xA0", strlen($m[0]));'), $out);
    // Unfortunately, IE won't preserve the newlines on clipboard copying even
    // with the following line.
    $out = preg_replace('/\n(?!<\/div>)/', '<br />', $out);

    if (!$return_only) {
      $this->out .= $out;
      $this->counter++;
    }

    return $out;
  }

  /**
   * Adds GeoGebra code.
   *
   * param $def
   *   Pure GeoGebra code ("definition"); In other words, do not HTML-escape
   *   before calling this method.
   */
  function addDefinition($def, $hidden = false) {
    if ($hidden) {
      $def = '(' . $def . ')';
    }

    if (strpos($def, '[') !== FALSE || strpos($def, '{') !== FALSE) {
      $expanded = Formatter::pp(new Scanner($def));
    }
    else {
      $expanded = $def;
    }

    if ($expanded != $def) {

      $collapsed = Highlighter_GG::run($def);
      $expanded  = Highlighter_GG::run($expanded);

      $id = 'row-' . $this->counter;
      $this->out .= '<div class="row-container" id="' . $id . '">';

      if ($this->initial_state == 'expanded') {
        $this->js_states[$id]['collapsed'] = $this->addRow($collapsed, 'collapsed', TRUE, TRUE);
        $this->js_states[$id]['expanded']  = $this->addRow($expanded, 'expanded', TRUE);
        if (!$this->send_both_states) {
          unset($this->js_states[$id]['expanded']);
        }
      }
      else {
        $this->js_states[$id]['expanded']  = $this->addRow($expanded, 'expanded', TRUE, TRUE);
        $this->js_states[$id]['collapsed'] = $this->addRow($collapsed, 'collapsed', TRUE);
        if (!$this->send_both_states) {
          unset($this->js_states[$id]['collapsed']);
        }
      }

      $this->out .= '</div>';
    }
    else {
      $this->addRow(Highlighter_GG::run($def));
    }
  }

  function addEmptyLine() {
    $this->addDefinition('');
  }

  function addScript($block, $language) {
    $block = rtrim($block);
    if ($language == 'GeoGebra') {
      foreach (explode("\n", $block) as $def) {
        $this->addDefinition($def);
      }
    }
    else {
      $this->addRow(($language == 'JavaScript') ? Highlighter_JS::run($block) : htmlspecialchars($block));
    }
  }

  function addEvent($event, $block, $language) {
    $this->addDefinition("$event = {");
    $this->addIndent();
    $this->addIndent();
    $this->addEmptyLine();
    $this->addScript($block, $language);
    $this->addEmptyLine();
    $this->removeIndent();
    $this->removeIndent();
    $this->addDefinition("}");
  }

  function beginMacro($name, $signature, $help) {
    $this->out .= sprintf('<div class="macro" id="macro__%s">', $name);
    Highlighter_GG::add_macro_name($name);
    if ($help) {
      $this->addDefinition("/* $help */");
    }
    $this->addDefinition($signature);
    $this->addIndent();
  }

  function endMacro() {
    $this->removeIndent();
    $this->addDefinition('end');
    $this->out .= '</div>';
  }

  static public function stringify($obj) {
    $output = new self();
    $obj->write($output);
    return $output->toString();
  }
}
