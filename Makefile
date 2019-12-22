
OBJS := checkPeel.o dataLoad.o maximcrc.o peelBinarySink.o infer.o peel.o
CXXFLAGS= -fopenmp -O2 -std=c++11 -Wall -g -I ../OpenQM
# 
# link
checkPeel: $(OBJS)
	g++ $(OBJS) -o $@ $(CXXFLAGS)

TIOBJS := infer.o testInference.o peel.o
testInference: $(TIOBJS)
	g++ $(TIOBJS) -o $@  $(CXXFLAGS)

SCOBJS := scoreConfs.o peel.o peelBinarySink.o infer.o
scoreConfs: $(SCOBJS)
	g++ $(SCOBJS) -o $@  $(CXXFLAGS)

TCOBJS := dataLoad.o trivialComb.o maximcrc.o ../OpenQM/implicant.o ../OpenQM/qm.o
trivialComb: $(TCOBJS) 
	g++ $(TCOBJS) -o $@  $(CXXFLAGS)

FEOBJS := flipExtractor.o dataLoad.o maximcrc.o
flipExtractor: $(FEOBJS)
	g++ $(FEOBJS) -o $@ $(CXXFLAGS)

CVOBJS :=convVcd.o dataLoad.o maximcrc.o
convVcd: $(CVOBJS)
	g++ $(CVOBJS) -o $@ $(CXXFLAGS)


all: checkPeel testInference scoreConfs trivialComb flipExtractor

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
%.o: %.cpp
	g++ -c $(CXXFLAGS) $*.cpp -o $*.o
	g++ -MM $(CXXFLAGS) $*.cpp > $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

# remove compilation products
clean:
	rm -f checkPeel $(OBJS) $(TIOBJS) $(SCOBJS) $(TCOBJS) $(FEOBJS) $(CVOBJS) *.d


