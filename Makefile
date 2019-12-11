
OBJS := checkPeel.o simul.o peelFileSink.o
CXXFLAGS= -fopenmp -O2 -std=c++11 -Wall
# 
# link
checkPeel: $(OBJS)
	g++ $(OBJS) -o $@ $(CXXFLAGS)

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

# compile and generate dependency info;
# more complicated dependency computation, so all prereqs listed
# will also become command-less, prereq-less targets
#   sed:    strip the target (everything before colon)
#   sed:    remove any continuation backslashes
#   fmt -1: list words one per line
#   sed:    strip leading spaces
#   sed:    add trailing colons
%.o: %.c
	g++ -c $(CXXFLAGS) $*.c -o $*.o
	g++ -MM $(CXXFLAGS) $*.c > $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

# remove compilation products
clean:
	rm -f checkPeel $(OBJS) *.d


