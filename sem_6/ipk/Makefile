CURDIR=`pwd`
SOURCEDIR=$(CURDIR)/src
CLASSPATHS=$(SOURCEDIR)

build:
	javac $(SOURCEDIR)/*.java
	
run: build
	java -classpath $(CLASSPATHS) Main ${PORT}
clean:
	rm $(SOURCEDIR)/*.class
