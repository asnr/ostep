# This file is a terrible hack to make spacemacs' compilation keyboard shortcuts
# work with this repo's structure. 'SPC p c' runs make at the project root and
# 'SPC c C' runs make in the current buffers directory, but the Makefile I want
# to run is between these directories :(

%:
	cd ../.. && make $(MAKECMDGOALS)
