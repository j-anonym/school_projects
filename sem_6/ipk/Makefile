CURDIR=`pwd`
SOURCEDIR=$(CURDIR)/src
CLASSPATHS=$(SOURCEDIR):$(SOURCEDIR)/socket_com/lib/json-20180813.jar

build:
	javac -classpath $(CLASSPATHS) src/socket_com/Main.java
	
run: build
	java -classpath $(CLASSPATHS) socket_com.Main ${api_key} ${city}
clean:
	rm *.class
