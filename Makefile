CPPC=g++
CPPFLAGS=-Wall -Wextra -Weffc++ -pipe -Ofast --std=c++11 -march=native -Wl,-O1 -Wl,-z,relro -Wl,-z,now -flto -DNDEBUG
OMP=-fopenmp

.PHONY: all clean 

all: gen pesma-limit4

%.o: %.cpp %.h
	$(CPPC) $(CPPFLAGS) -c $< -o $@

common.o: common.cpp common.h
	$(CPPC) $(CPPFLAGS) $(OMP) -c $< -o $@

flatinvmap.o: flatinvmap.cpp flatinvmap.h bloom_filter.h
	$(CPPC) $(CPPFLAGS) -c $< -o $@

gen: gen.cpp common.o
	$(CPPC) $(CPPFLAGS) $(OMP) $^ -o $@

agent4.o: agent4.cpp agent4.h flatinvmap.h
	$(CPPC) $(CPPFLAGS) -c $< -o $@

pesma-limit4.o: pesma-limit4.cpp agent4.h common.h bloom_filter.h
	$(CPPC) $(CPPFLAGS) $(OMP) -c $< -o $@

pesma-limit4: pesma-limit4.o agent4.o common.o flatinvmap.o bloom_filter.o
	$(CPPC) $(CPPFLAGS) $(OMP) $^ -o $@

clean: 
	$(RM) gen pesma-limit4
	$(RM) *.o
