EXENAME := toggle_fullscreen

all: $(EXENAME) Makefile

$(EXENAME): main.cpp
	gcc main.cpp -lX11 -std=c++11 -fno-exceptions -o $(EXENAME)

install: $(EXENAME) Makefile
	@cp $(EXENAME) /usr/local/bin/$(EXENAME) && echo "Installed"

uninstall:
	@rm -f /usr/local/bin/$(EXENAME)

# make run WINDOW_NAME
run: $(EXENAME)
	./$(EXENAME) $(filter-out $@,$(MAKECMDGOALS))
