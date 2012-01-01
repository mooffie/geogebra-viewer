
function bm_entry() {

  if (location.href.indexOf('~mooffie/gg') != -1) {
    alert("No, that's not the way to use this link. First drag this link to your toolbar, or bookmark it. Then call it up when you're on a page showing a forum message.");
    return;
  }

  /**
   * Turns a relative URL into an absolute one.
   *
   * Code taken from http://stackoverflow.com/questions/470832/getting-an-absolute-url-from-a-relative-one-ie6-issue
   */
  var qualifyURL = function(url) {
    var escapeHTML = function (s) {
      return s.split('&').join('&amp;').split('<').join('&lt;').split('"').join('&quot;');
    }
    var el = document.createElement('div');
    el.innerHTML= '<a href="'+escapeHTML(url)+'">x</a>';
    return el.firstChild.href;
  }

  var fixURL = function(url) {
    // Remove the session ID for anonymous users.
    url = url.replace(/&sid=[^&;]+/, '');
    return url;
  }

  /**
   * Creates the IMG and IFRAME tags and inserts them into the document.
   */
  var addDom = function(url, parent) {

    var SERVICE = 'http://www.typo.co.il/~mooffie/ggb/thumb.php';

    var iframe = document.createElement('IFRAME');
    iframe.style.minWidth = '300px';
    iframe.width = '50%';
    iframe.height = '300';
    iframe.src = SERVICE + '?url=' + escape(url) + '&output_type=txt';
    iframe.style.display = 'none';

    var img = document.createElement('IMG');
    img.alt = 'Loading ...';
    img.border = '1';
    img.onload = function() {
      // If the image is tiny, enlarge to 300px. If it's huge, shrink to 600px.
      var currentSize = Math.max(this.width, this.height);
      var newSize     = Math.min(Math.max(currentSize, 300), 600);
      var ratio = newSize / currentSize;
      // "if only a width or a height is specified, the image will be scaled automatically and retain its aspect ratio."
      this.width *= ratio;
      iframe.style.display = 'inline-block';
    };
    img.src = SERVICE + '?url=' + escape(url);

    // GeogebraTube.org has an image at the background that gets on top of our iframe.
    parent.style.backgroundImage = 'none';
    // The following is to prevent cut in pages like https://sites.google.com/site/ggbgv11/home/geogebra-files
    parent.style.overflow = 'auto';

    parent.appendChild(img);
    parent.appendChild(iframe);
  }

  /**
   * Finds a container in which to inject our DOM.
   */
  function findContainer(elt) {
    while (elt && elt.tagName && !/^(td|div|p)$/i.test(elt.tagName)) {
      elt = elt.parentNode;
    }
    // As last resort, take <body>.
    return (elt && elt.tagName) ? elt : document.body;
  }

  /**
   * Reads a <param> value of an applet.
   */
  var getAppletParam = function(elt, name) {
    for (var list = elt.getElementsByTagName('param'), i = 0; i < list.length; i++) {
      var param = list[i];
      if (param.getAttribute('name') == name) {
        return param.getAttribute('value');
      }
    }
    return null;
  }

  var s = '';
  var counter = 0;

  //
  // Handle links.
  //
  // Originally, links we checked only in pages contianing 'viewtopic' in their
  // URL, to prevent abuse. Perhaps in the future we'll want to re-introduce
  // some anti-abuse measure.
  for (var list = document.getElementsByTagName('a'), i = 0; i < list.length; i++) {
    var a = list[i];
    if (/\.gg[bt]/i.test(a.href + a.innerHTML)) {
      counter++;
      // According to http://www.w3.org/TR/DOM-Level-2-HTML/html.html#ID-48250443 , the 'href' property is an *absolute* URI.
      s += "\n" + fixURL(a.href);
      addDom(fixURL(a.href), findContainer(a));
    }
  }

  //
  // Handle applets.
  //
  for (var list = document.getElementsByTagName('applet'), i = 0; i < list.length; i++) {
    var a = list[i];

    var url = null;
    if (getAppletParam(a, 'filename')) {
      url = qualifyURL(getAppletParam(a, 'filename'));
    }
    else if (getAppletParam(a, 'ggbBase64')) {
      url = location.href + ';' + i;
    }

    if (url) {
      counter++;
      s += "\n" + url;
      addDom(fixURL(url), findContainer(a));
    }
  }

  //
  // Handle geogebratube.org intro pages.
  //
  // No, we're temporary(?) disabling this. These page already contain a "download" link,
  // so no need for special treatment.
  //
  /*if (/material\/show\/id\/(\d+)/.test(location.href)) {
    var url = 'http://www.geogebratube.org/files/material-' + RegExp.$1 + '.ggb';
    addDom(url, findContainer(document.getElementById('material-actions') || document.getElementById('link-thumbnail')));
    counter++;
  }*/

  if (counter == 0) {
    var msg = "I don't see any links to GeoGebra files (*.ggb), nor GeoGebra applets, on this page. There's nothing for me to do here.";
    alert(msg);
  }
  //alert(s);
}

bm_entry();
