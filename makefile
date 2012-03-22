all: import

import:
	gimptool-2.0 --install import.c

gitpush:
	git push --all https://code.google.com/p/gimp-isptool/
