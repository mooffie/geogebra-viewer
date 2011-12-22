#!/usr/bin/ruby1.9.1

def compact_javascript(s)
  # Remove comments.
  s.gsub!(%r|(?<!http:)//[^\n]*|, '');
  s.gsub!( %r|/\* .*? \*/|mx, '');

  # Trim lines.
  s.gsub!( / ^[ \t]+ | [ \t]+$ /x, '');

  # To be able to fold line, a line must must end in "[ { } , ;" only (unless "}"
  # or "]" follow on the next line). We make sure this is the case.
  s.scan( / [^\[{},;\n] \n (?!\s*[}\]]) /x ) {
    line_no = $`.count("\n")
    line = s.lines.to_a[line_no].chomp
    raise "Error at line #{line_no+1} ('#{line}'): it doesn't finish with a 'good' character so I can't fold it.";
  }

  # Compressions. Some of this may not work for your script.
  s.gsub!(' = ', '=');
  s.gsub!(' == ', '==');
  s.gsub!(' - ', '-');
  s.gsub!(' + ', '+');
  s.gsub!(', ', ',');
  s.gsub!(': ', ':');
  s.gsub!(' (', '('); # e.g., "for (...)"
  s.gsub!(' {', '{'); # e.g., "function xyz() {"

  # Finally, remove newlines.
  s.gsub!("\n", '');
end

script = File.read(ARGV[0] || (raise "please provide a filename"))
script = compact_javascript(script)
# We can't use 'puts' because we don't want the trailing \n
$stdout.write script

#File.open('out.js', 'w') { |f|
#  f.write(script);
#}

if script[' '] || script["\t"]
#  raise "The compressed JavaScript still contains whitespace. Some say this can cause troubles."
end
