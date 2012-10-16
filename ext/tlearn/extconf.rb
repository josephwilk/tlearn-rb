require 'mkmf'

# compiler optimization level
OPTIMIZE= '-O'		

RbConfig::MAKEFILE_CONFIG['CC'] = 'gcc'

$CFLAGS = "#{OPTIMIZE}"
$LFLAGS= '-abcn -Dlint'

$DESTDIR= '.'
$LIBS = '-lm' 

create_makefile('tlearn/tlearn')