<html>
<style type="text/css">
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

.news {
  border: 1px dashed black;
  font-size: smaller;
  background-color: #f3d699;
  padding: 0.3em;
}

fieldset {
  margin-top: .5em;
  margin-bottom: .8em;
  padding-top: .8em;
  padding-bottom: .8em;
}

fieldset legend {
  font-style: italic;
  font-weight: bold;
}

.error {
  color: red;
  font-weight: bold;
  margin: 1em 0;
}

</style>
<body>

<h1>GeoGebra thumbnails: bookmarklet</h1>

<p>The GeoGebra forums, and web-pages in general, have a little problem:
You have to launch GeoGebra to see the .GGB worksheets attached to
messages. There's a similar problem with applets. That's a drag if
you're using a slow computer.</p>

<p>This page contains something to alleviate the problem: a
bookmarklet<sup><a
href="http://en.wikipedia.org/wiki/Bookmarklet">[?]</a></sup> to show
you the worksheets' thumbnails (and construction protocol). <strong>Drag the
following link to your browser's toolbar</strong> (<strong>or bookmark it</strong>
with your right mouse button). Then, when you're browsing a forum page,
or any page containing links to *.ggb files, or a page containing
applets, click the button to see the thumbnails.</p>

<p class="blet">
<a href="javascript:(function(){if(!window.bm_entry){_my_script=document.createElement('SCRIPT');_my_script.type='text/javascript';_my_script.src='http://www.typo.co.il/~mooffie/ggb/bookmarklet-app.js?x='+Math.random();document.getElementsByTagName('head')[0].appendChild(_my_script);}else{alert('The bookmarklet was already used on this page.');}})();">Show GG thumbs</a>
</p>

<p class="news"><strong>NEWS:</strong> As of 2012-January-02, the
webmasters of GeoGebra's forum have kindly incorporated this code into
the forum (you'll see a "<strong>Show thumbnails</strong>" link there).
So you no longer need to use this bookmarklet there (in fact, you
can't). But it's still very useful when you browse other pages on the
internet.</p>

<p>Here's a screenshot of this in action (click to enlarge):</p>

<div class="screenshot">
<a href="blet.png"><img src="blet_th.jpg" /></a>
</div>

<p>Two things are encircled in red here: the toolbar button, and the
thumbnail (+ construction protocol) you get after clicking it.</p>

<p><a name="applets">For applets</a>, seen below, there's an additional
feature: a "[download]" link that points to the .ggb file. In other
words, you no longer have to use your browser's Java system; you can use
this link to launch your local GeoGebra. (This also works for base64
applets.)</p>

<div class="screenshot">
<a href="applet.png"><img src="applet_th.jpg" /></a>
</div>

<h2>Limitations</h2>

<ul>

<li><strong>Thumbnails availability</strong>: This application doesn't
create the thumbnails itself but extracts the ones created by GeoGebra.
So if the worksheet was created with an old version of GG (3.01, 2.0,
etc.) you won't get a thumbnail. Base64 applets too don't have a
thumbnail.</li>

<li><strong>Network connectivity</strong>: It's this server (typo.co.il)
that downloads/analyzes your worksheets, not your browser. So if my
server can't access the worksheet (e.g., if it's behind a login system
(user name / password)) the operation will fail.</li>

<li>Applets inside an <strong>iframe</strong> aren't recognized (A
workaround: most browsers let you open an iframe in a new tab; click
with your right mouse button in the frame and see the menu).</li>

</ul>

<h2>Uploading a file</h2>

<p>If you wish to analyze your own ggb (or ggt) files, that aren't on the internet, you may upload them here.</p>

<form method="post" enctype="multipart/form-data">

<fieldset>
<legend>Upload your own file</legend>

<?php
require_once 'utils.inc';

date_default_timezone_set('Asia/Jerusalem'); // We have to set something or else PHP will complain.

if (!empty($_FILES['file'])) {

  $MAX_SIZE = 1024 * 1024 * 10;

  $error = '';
  if ($_FILES['file']['error'] > 0) {
    switch ($_FILES['file']['error']) {
      case UPLOAD_ERR_NO_FILE:  $error = 'No file was uploaded.'; break;
      case UPLOAD_ERR_INI_SIZE: $error = 'File too big.'; break;
      default: $error = sprintf('Some error occured (%s).', $_FILES['file']['error']);
    }
  }
  else if ($_FILES['file']['size'] > $MAX_SIZE) {
    $error = sprintf('File too big for uploading (max %s bytes allowed here).', $_FILES['file']['size']);
  }
  else if (!is_valid_zip($_FILES['file']['tmp_name'])) {
    $error = "This doesn't look like a valid ggb/ggt file (that is, it's not a valid ZIP file).";
  }

  function service_url($file, $output_type) {
    $file_url = absolutize_url($file);
    return sprintf('http://www.typo.co.il/~mooffie/ggb/thumb.php?url=%s&output_type=%s', urlencode($file_url), $output_type);
  }

  if (!$error) {
    $outfile = 'tmp/user_upload' . date('__Y-m-d__H_i_s__') . rand() . '.ggb';
    move_uploaded_file($_FILES['file']['tmp_name'], $outfile);
    header('Location: ' . service_url($outfile, $_REQUEST['output_type']));
  }

  if ($error) {
    echo "<div class='error'>Error: $error</div>";
  }
}
?>

The file: <input type="file" name="file" />

<fieldset>

<legend>What do you wish to see?</legend>
  <input type="radio" name="output_type" id="show-html" value="html" checked="checked" />
    <label for="show-html">The construction protocol</label> <br />
  <input type="radio" name="output_type" id="show-plain" value="txt" />
    <label for="show-plain"> The construction protocol (plain text version)</label> <br />
  <input type="radio" name="output_type" id="show-png" value="png" />
    <label for="show-png">The thumbnail (if exists)</label>
</fieldset>

<input type="submit" name="submit" value=" Upload! " />

</fieldset>

</form>

<h2>Contact</h2>

<p>In you encounter any problems, <a href="mailto:mooffie@gmail.com">email me</a> or
ask <a href="http://www.geogebra.org/forum/viewtopic.php?f=44&t=25201">in the forum</a>.
The source code can be found <a href="https://gitorious.org/geogebra-thumbnails-web-extractor/geogebra-thumbnails-web-extractor/trees/master">here</a>.</p>
