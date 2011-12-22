
This is the source code for the GeoGebra thumbnail+protocol extractor.
Lisenced under GPL (in short: it's free).

This is a PHP application. You should install it on a unix based
server (because it executes the 'find' and 'unzip' commands).

Installation instructions:

- Create a 'tmp' and 'cache' folders. Chmod them 0777 or else the server
  won't be able to write there.

- Update the URLs in 'index.html' and 'bookmarklet-app.js' to point to
  your own server.

