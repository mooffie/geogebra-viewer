
# Run this to generate .html.gz for every .html file. But you don't normally
# need to run this: the thumbnailer already runs 'gzip' when it finishes.

for h in cache/*.html; do echo $h ...; gzip -c "$h" > "$h".gz; done
