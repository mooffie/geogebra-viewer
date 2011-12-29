
function bm_entry() {

  if (location.href.indexOf('~mooffie/gg') != -1) {
    alert("No, that's not the way to use this link. First drag this link to your toolbar, or bookmark it. Then call it up when you're on a page showing a forum message.");
    return;
  }

  if (location.href.indexOf('viewtopic') == -1) {
    alert('You can use this bookmarklet only on a page showing a forum message.');
    return;
  }

  var fix_url = function(url) {
    // Remove the session ID for anonymous users.
    url = url.replace(/&sid=[^&]+/, '');
    return url;
  }

  var SERVICE = 'http://www.typo.co.il/~mooffie/ggb/thumb.php';

  /**
   * Creates the IMG and IFRAME tags and inserts them into the document.
   */
  var add_dom = function(url, parent) {
    var iframe = document.createElement('IFRAME');
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

    parent.appendChild(img);
    parent.appendChild(iframe);
  }

  var s = '';
  var counter = 0;

  for (var list = document.getElementsByTagName('a'), i = 0; i < list.length; i++) {
    var a = list[i];
    if (a.innerHTML.toLowerCase().indexOf('.gg') == -1) { // .gg catches both .ggb and .ggt
      continue;
    }
    if ((a.getAttribute('href') || '').indexOf('download') != -1) {
      counter++;
      // According to http://www.w3.org/TR/DOM-Level-2-HTML/html.html#ID-48250443 , the 'href' property is an *absolute* URI.
      s += "\n" + fix_url(a.href);
      var parent = a;
      while (parent && parent.tagName.toLowerCase() != 'td' && parent.tagName.toLowerCase() != 'div') {
        parent = parent.parentNode;
      }
      add_dom(fix_url(a.href), parent ? parent : document.body);
    }
  }

  if (counter == 0) {
    alert('No GGB files were found on this page.');
  }
  //alert(s);
}

bm_entry();
