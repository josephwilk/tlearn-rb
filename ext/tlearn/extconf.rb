require 'mkmf'

# compiler optimization level
OPTIMIZE= '-O'		

RbConfig::MAKEFILE_CONFIG['CC'] = 'gcc'
RbConfig::MAKEFILE_CONFIG['DLEXT'] = 'so'

$CFLAGS = "#{OPTIMIZE}"
$LFLAGS= '-abcn -Dlint'

$DESTDIR= '.'
$LIBS = '-lm' 

create_makefile('tlearn/tlearn')