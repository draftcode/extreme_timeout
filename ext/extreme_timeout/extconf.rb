require 'mkmf'
have_func('backtrace', 'execinfo.h')
create_makefile('extreme_timeout')
