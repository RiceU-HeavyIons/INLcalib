#CXX     = clang++
CXX     = g++
INC     = ${ROOTSYS}/include/
LIBS     = ${ROOTSYS}/lib/
CXXFLAG = `root-config  --cflags` `root-config  --libs`
# -std=c++11 -O2  -lCore  -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -lm -ldl -rdynamic  -lGeom 

all:		inl_analysis corr_analysis cosmic_corr_analysis cosmic_corr_analysis1 cosmic_corr_analysis2 summary_analysis checkdata checkmissword cosmic_analysis noise_analysis start_noise_analysis inlsummary_analysis ncorr_analysis oneinl_analysis onecorr_analysis RCcorr_analysis noRCcorr_analysis inlstudy_analysis

inlstudy_analysis:	inlstudy_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
inl_analysis:	inl_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
corr_analysis:	corr_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
cosmic_corr_analysis:	cosmic_corr_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
cosmic_corr_analysis1:	cosmic_corr_analysis1.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
cosmic_corr_analysis2:	cosmic_corr_analysis2.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
ncorr_analysis:	ncorr_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
summary_analysis:	summary_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
inlsummary_analysis:	inlsummary_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
checkdata:	checkdata.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
checkmissword:	checkmissword.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
cosmic_analysis:	cosmic_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
noise_analysis:	noise_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
start_noise_analysis:	start_noise_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
misc_analysis:	misc_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
oneinl_analysis:	oneinl_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
onecorr_analysis:	onecorr_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
RCcorr_analysis:	RCcorr_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
noRCcorr_analysis:	noRCcorr_analysis.cxx
	$(CXX) -Wall $^ -o $@ -L${ROOTSYS}/lib/ $(CXXFLAG) -I$(INC)
clean:
	rm -f *.o inl_analysis corr_analysis cosmic_corr_analysis cosmic_corr_analysis1 cosmic_corr_analysis2 summary_analysis checkdata checkmissword cosmic_analysis noise_analysis start_noise_analysis inlsummary_analysis ncorr_analysis oneinl_analysis onecorr_analysis RCcorr_analysis noRCcorr_analysis inlstudy_analysis
#	rm -f *.o inl_analysis corr_analysis cosmic_corr_analysis cosmic_corr_analysis1 cosmic_corr_analysis2 summary_analysis checkdata checkmissword noise_analysis misc_analysis
