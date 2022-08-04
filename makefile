Parser=parser
DUG=debug
HTTP_SEARCHER=http_searcher
cc=g++

.PHONY:all
all:$(Parser) $(DUG) $(HTTP_SEARCHER)
$(Parser):parser.cc
	$(cc) -o $@ $^ -lboost_system -lboost_filesystem -std=c++11
$(DUG):debug.cc
	$(cc) -o $@ $^  -ljsoncpp -std=c++11
$(HTTP_SEARCHER):http_searcher.cc
	$(cc) -o $@ $^  -ljsoncpp -lpthread -std=c++11

	
.PHONY:clean
clean:
	rm -rf $(Parser) $(DUG) $(HTTP_SEARCHER)
