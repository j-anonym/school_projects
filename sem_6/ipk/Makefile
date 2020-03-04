CURDIR=`pwd`
SOURCEDIR=$(CURDIR)/src
CLASSPATH=$(SOURCEDIR)

build:
	javac $(SOURCEDIR)/*.java
	
run: build
	java -classpath $(CLASSPATH) Main ${PORT}
clean:
	rm $(SOURCEDIR)/*.class
zip:
	zip xvavro05.zip src/*.java Makefile readme.md