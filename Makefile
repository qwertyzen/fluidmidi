clean:
	rm -rf fluidmidi/pyside/*.c \
	fluidmidi/pyside/*.so \
	fluidmidi/*.c fluidmidi/*.so

uninst:
	pip uninstall fluidmidi -y

install: clean uninst
	pip install -v . --no-build-isolation

install-e: clean uninst
	pip install -v -e . --no-build-isolation
