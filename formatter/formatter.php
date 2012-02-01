<html>
<head>
<meta http-equiv="Content-Type" content="text/ html; charset=UTF-8" />
<title>GeoGebra Formatter</title>
<link rel="search" type="application/opensearchdescription+xml" title="GoeGebra Formatter" href="formatter-opensearch.xml" />
<style type="text/css">

/* General CSS: taken from the thumbnailer homepage. */

body {
  margin-left: 3em;
  margin-right: 4em;
  margin-top: 2em;
}

h1,h2,h3,h4 {
  margin-left: -0.2em;
}

p.blet {
  margin-top: 2em;
  margin-bottom: 2em;
  text-align: center;
}

div.screenshot {
  text-align: center;
}

div.screenshot a {
  border: none;
}

li {
  margin-top: .9em;
}

/* CSS specific to this document: */

h1,h2,h3,h4 {
  font-family: sans-serif;
  margin-left: -0.5em;
  color: #005A9C;
}

body {
  font-family: sans-serif;
}

textarea {
  font-size: 13px;
  padding: 0.5em 0 0.5em 0.5em;
  width: 100%;
  margin-bottom: 0.6em;
}

textarea, input.url {
  background-color: #FFCC99;
}

td.source {
  vertical-align: top;
  width: 40%;
}

td.target {
  vertical-align: top;
}

td.buttons {
  text-align: center;
}

input.clear-button {
  font-weight: bolder;
  font-size: larger;
}

div.clear-button {
  text-align: center;
}

label {
  font-weight: bolder;
  display: inline-block;
  margin-bottom: 0.5em;
}

code.url {
  display: inline-block;
  border: 1px dashed black;
  padding: 0.2em 0.6em;
}

.url-examples {
  font-size: xx-small;
}

</style>

<script type="text/javascript">
function $$(id) {
  return document.getElementById(id);
}

function show_url(url) {
  $$('url').value = url;
  // We use setTimeout() for the URL to be seen updated in the form.
  window.setTimeout(function() {
    $$('fetch').click();
  },1);
}
</script>

</head>

<body>

<h1>GeoGebra Formatter</h1>

<p>One problem with GeoGebra is that sometimes you have to deal with
long long loooong definitions that are all on single lines. Sometimes
it's hard to read, or create, these definitions -- especially when they
contain nested commands and lists. </p>

<p>This tool solves this problem. Enter your one-line definition(s)
below. Then click the right-pointing arrow button. You'll then see a
nicely-formatted result in the box on the right.</p>

<p>You can also go the opposite direction: click the left-pointing
arrow to convert nice text into one-line GeoGebra definitions.</p>

<p>(Click the "Clear!" button to remove the examples shown.)</p>

<?php

require 'pp.inc';

$default =<<< EOS
// The following are example definitions
// taken from real worksheets.
//
// Note: The blank lines between the
// definitions are for clarity only.

Liste2_1 = Sequence[Polygon[(a + k * Δx, 0), (a + (k + 1) * Δx, 0), (a + (k + 1) * Δx, f(a + (2 * k + 1) * Δx / 2)), (a + k * Δx, f(a + (2 * k + 1) * Δx / 2))], k, 0, n - 1]

liste2 = Sequence[Beziers[Element[liste1, k], If[k ≠ 1, Element[liste1, k], 0.5 * (Element[liste1, 1] + Element[liste1, 2])] + If[k ≟ 1, 0, 1] * 1 / 3 * (x(Element[liste1, k + 1] - Element[liste1, k]), 0), If[k ≠ Length[liste1] - 1, Element[liste1, k + 1], 0.5 * (Element[liste1, Length[liste1] - 1] + Element[liste1, Length[liste1]])] + If[k ≟ Length[liste1] - 1, 0, -1] * 1 / 3 * (x(Element[liste1, k + 1] - Element[liste1, k]), 0), Element[liste1, k + 1]], k, 1, Length[liste1] - 1]

R1numtext = Element[{"AB", "BC", "CA", "A'B'", "B'C'", "C'A'", "?"}, IndexOf[signature, {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {1, 0, 1, 0, 0, 0}, {1, 1, 0, 1, 1, 0}, {0, 1, 1, 0, 1, 1}, {1, 0, 1, 1, 0, 1}, signature}]]

LTeilPolynome = Sequence[Function[Polynomial[{Element[LPunkte, j], Element[LPunkte, j + 1], Element[LPunkte, j + 2]}], Element[LxWerte, j], Element[LxWerte, j + 2]], j, 1, 2 * n - 1, 2]

ww = If[theta < 0.5 * π, 20.0, If[theta < π, 5.0, If[theta < 2.0 * π, 3.0, 1.0]]]

Fi0 = Sequence[Sum[Sequence[Element[Q01, i] / Segment[Element[S11, j], Element[P01, i]], i, 1, N_Q]], j, 1, nx + 1]

liste1 = If[y(C - B) * y(B - A) > 0, {G, B, If[Defined[x(Element[liste2, 1])], Element[liste2, 1], If[Defined[x(Element[liste2, 2])], Element[liste2, 2], (0, 0)]]}, {E, B, J}]

cronoMarks = {Sequence[Segment[cronoP0 + (2 * scale; s°), cronoP0 + (1.9 * scale; s°)], s, 0, 360, 6], Sequence[Segment[cronoP0 + (2 * scale; s°), cronoP0 + (1.75 * scale; s°)], s, 0, 360, 30], Sequence[Segment[cronoP2 + (0.6 * scale; s°), cronoP2 + (0.55 * scale; s°)], s, 0, 360, 6], Sequence[Segment[cronoP2 + (0.6 * scale; s°), cronoP2 + (0.5 * scale; s°)], s, 0, 360, 30]}

crono = "" + (If[hours < 1, "00", If[hours < 10, "0" * hours, "" * hours]]) + ":" + (If[minutes < 1, "00", If[minutes < 10, "0" * minutes, "" * minutes]]) + ":" + (If[seconds < 1, "00", If[seconds < 10, "0" * seconds, "" * seconds]]) + "." + miliseconds + ""

EOS;
$default_url = '';

//S_Command::$indent_string = '  ';
$source = isset($_REQUEST['source']) ? $_REQUEST['source']    : $default;
$target = isset($_REQUEST['target']) ? $_REQUEST['target']    : '';
$url    = isset($_REQUEST['url'])    ? trim($_REQUEST['url']) : $default_url;
$is_backward = !empty($_REQUEST['backward']);

if (!empty($_REQUEST['fetch'])) {
  if (empty($url)) {
    $source = "You didn't fill in the URL.";
  }
  else {
    if (!preg_match('/^http/i', $url)) {
      $source = "This URL seems invalid.";
    }
    else {
      $service = sprintf('http://www.typo.co.il/~mooffie/ggb/thumb.php?url=%s&output_type=plain', urlencode($url));
      if (!($source = @file_get_contents($service))) {
        $source = "This doesn't look like a GeoGebra file.\nMake sure you typed the URL correctly.";
      }
    }
  }
}

if ($is_backward) {
  $source = S_Utils::unPp($target);
}
else {
  $target = S_Utils::pp($source);
}

?>

<form method="post">
<table b-order="1" width="100%">
  <tr>

    <td class="source">
      <label for="source">Enter your one-line definition(s) here:</label> <br />
      <textarea name="source" id="source" cols="40" rows="25" wrap="off" spellcheck="false"><?php echo htmlspecialchars($source); ?></textarea>
      <br />
      <div class="clear-button">
        <input type="button" class="clear-button" value="Clear!" onclick="document.getElementById('source').value = ''; document.getElementById('target').value = '';" />
      </div>
    </td>

    <td class="buttons">
       <input name="forward" type="submit" value="  --&gt;  " />
       <br />
       <br />
       <br />
       <br />
       <input name="backward" type="submit" value="  &lt;--  " />
    </td>

    <td class="target">
      <label for="target">Nicely-formatted output:</label> <br />
      <textarea name="target" id="target" cols="40" rows="25" wrap="off" spellcheck="false"><?php echo htmlspecialchars($target); ?></textarea>
    </td>

  </tr>
</table>

<p>
<label>Alternatively, type the URL of some .ggb (or .ggt) file whose protocol you wish to format:</label>
<input type="text" name="url" id="url" class="url" value="<?php echo htmlspecialchars($url); ?>" size="80" /> <input type="submit" name="fetch" id="fetch" value="Go!" />
<span class="url-examples">
<a href="javascript:void show_url('http://www.geogebra.org/forum/download/file.php?id=12494')">example1</a>,
<a href="javascript:void show_url('http://www.geogebra.org/forum/download/file.php?id=12192')">example2</a>,
<a href="javascript:void show_url('http://www.geogebra.org/forum/download/file.php?id=12884')">example3</a>,
<a href="javascript:void show_url('http://www.geogebra.org/forum/download/file.php?id=12804')">example4</a>.
</p>

</form>

<h2>How to use this tool</h2>

<p>The obvious way to use this tool is to "copy" to clipboard your definition and then "paste" it into this page.

<p>However, you may find this way tedious. Most browsers provide a
shorcut: they let you mark the text (the GeoGebra definitions) with the
mouse and then, via the context menu, send it to some search engine. The
"trick" is to add this tool to the list of search engines your browser
knows about. Here it is in action:</p>

<div class="screenshot">
<a href="formatter.png"><img src="formatter_th.jpg" /></a>
</div>

<p>Here are instructions for the common browsers:

<h3>Browser: Opera</h3>

<ol>
<li>Go to "Preferences", then the "Search" tab, and "Add..." a
new search engine; make its URL
<code class="url">http://www.typo.co.il/~mooffie/ggb/formatter/?source=%s</code></li>
<li>That's all.</li>
</ol>

<h3>Browser: Firefox</h3>

<ol>

<li>Add this web-page to Firefox's list of search engines: The browser
has a search box, usualy in the upper right corner; Clicking its
dropdown menu would reveal a new item: "Add 'GeoGebra Formatter'". Click
this item. Here's a screenshot showing this (click to enlarge):<br />

  <a href="formatter-opensearch.png"><img src="formatter-opensearch_th.jpg" /></a>
</li>

<li>Install an extension that lets you search the selcted text; Examples:
  <del><a href="https://addons.mozilla.org/en-US/firefox/addon/context-search/">Context Search</a></del>
  or
  <a href="https://addons.mozilla.org/en-US/firefox/addon/selected-search/">Selected Search</a>.
  <small>(2012-Jan-17: The "Context Search" extension is superior, but it has an unfortunate bug: it's limited to 150 characters; I've sent its developer a note. Stay tuned.)</small>
</li>

</ol>

<h3>Browser: Google Chrome</h3>

<ol>
<li>Install an extension that lets you search the selcted text; Examples:
  <a href="https://chrome.google.com/webstore/detail/ocpcmghnefmdhljkoiapafejjohldoga">Context Menu Search</a>
  or
  <a href="https://chrome.google.com/webstore/detail/dbpfafcplnjmakknnonpegphpmpmhjhj">Context Search</a>.
</li>

<li>Add this web-page to your extension's list of search engines: Go to
the extension's "options" screen and enter a URL similar to the one
mentioned for the Opera browser (different extensions may have a
slightly different syntax, so I can't provide exact instuctions;
basically you'd change the <code class="url">%s</code> to some other
token).</li>

</ol>

<h3>Browser: Internet Explorer</h3>

<ol>
<li>Step #1 for Firefox is applicable to IE 7+ as well.</p>
<li>(I don't have access to IE so I don't know if anything more is needed.)</li>
</ol>

<h2>General notes about browser integration:</h3>

<ul>

<li>When using the context menu: Unfortunately, browsers don't send
newline characters to a "search engine". As a result, if you select
several definitions, this web-page will think it's a single definition
and the output won't be as pretty. So make sure to select only one
definition at a time.</li>

<li>When using the context menu: In some browsers/extensions you'll have
to hold Shift down while clicking for the tool to open in a separate
tab.</li>

</ul>

<h2>Contact</h2>

<p>In you encounter any problems, <a href="mailto:mooffie@gmail.com">email me</a><!-- or
ask <a href="http://www.geogebra.org/forum/viewtopic.php?f=XXXXX">in the forum</a>-->.</p>

</ul>
