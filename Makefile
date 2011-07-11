source = ascii2floats.cpp floats2ascii.cpp fft.cpp ifft.cpp scmanal.cpp \
	add.cpp multiply.cpp divide.cpp scale.cpp gpout.cpp dad.cpp \
	sinegen.cpp noisegen.cpp hanning.cpp normalize.cpp threshold.cpp spectro.cpp \
	aiffread.cpp
executables = ascii2floats floats2ascii fft ifft add multiply divide scale \
	gpout dad sinegen noisegen hanning normalize threshold spectro \
	aiffread

CPP = g++
INCLUDES =
CPPFLAGS = -O2 -g -Wall -pipe -pedantic -Werror -Wextra -std=c++0x
LIBS = -lrfftw -lfftw -lm -lsndfile

all: $(executables)

ascii2floats: ascii2floats.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o ascii2floats.cpp

floats2ascii: floats2ascii.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o floats2ascii.cpp

threshold: threshold.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o threshold.cpp

spectro: spectro.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o spectro.cpp

add: add.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o add.cpp

multiply: multiply.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o multiply.cpp

divide: divide.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o divide.cpp

scale: scale.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o scale.cpp

normalize: normalize.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o normalize.cpp

sinegen: sinegen.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o sinegen.cpp

noisegen: noisegen.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o noisegen.cpp

hanning: hanning.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o hanning.cpp

gpout: gpout.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o gpout.cpp

fft: fft.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o fft.cpp

ifft: ifft.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o ifft.cpp

dad: dad.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o dad.cpp

aiffread: aiffread.cpp scmanal.o
	$(CPP) $(INCLUDES) $(CPPFLAGS) $(LIBS) \
	-o $@ scmanal.o aiffread.cpp



include depend

depend: $(source)
	$(CPP) $(INCLUDES) -MM $(source) > depend

.PHONY: clean
clean:
	-rm -f $(executables) depend *.o
