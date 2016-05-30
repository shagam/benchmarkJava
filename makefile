
help:
	java -ea -jar dist/benchmarkJava.jar help
	#echo  benchmark.c
	~/nb/benchmarkJava/src/benchmarkJava/benchmark help 

# ARG=' help ' make java
# ARG=' testName=tree size=10000 thr=8 ' make java
# ARG=' testName=noLock thr=8 ' make java
# ARG=' testName=queue ' make java
# ARG=' testName=hash  ' make java
# ARG=' testName=alloc chunk=1000 thr=8 ' make java
# ARG=' testName=copy  ' make java
# ARG=' testName=prime ' make java
# ARG=' testName=count  thr=8 ' make java

java:
	java -ea -XX:+UseCompressedOops -XX:SurvivorRatio=1 -XX:+PrintGCDetails -XX:+PrintTenuringDistribution -Xloggc:/var/js/java.log_benchmark -jar dist/benchmarkJava.jar ${ARG}
java_pause:
	java -ea -XX:MaxGCPauseMillis=20 -XX:+PrintGCDetails -XX:+PrintGCApplicationStoppedTime -Xloggc:/var/js/java.log_benchmark -jar dist/benchmarkJava.jar ${ARG}


#https://blog.codecentric.de/en/2014/01/useful-jvm-flags-part-8-gc-logging/
#https://blog.codecentric.de/en/2013/01/useful-jvm-flags-part-6-throughput-collector/

#java -ea -XX:+UseConcMarkSweepGC -XX:+UseParNewGC  -XX:+PrintTenuringDistribution -verbose:gc  -Xloggc:/var/js/java.log_benchmark -jar dist/benchmarkJava.jar ${ARG}

# ARG=' help ' make c
# ARG=' testName=tree size=10000 thr=8 ' make c
# ARG=' testName=noLock ' make c
# ARG=' testName=queue ' make c
# ARG=' testName=hash  ' make c
# ARG=' testName=alloc chunk=1000 thr=8 ' make c
# ARG=' testName=copy  thr=8 ' make c
# ARG=' testName=prime size=1000 thr=8  ' make c
# ARG=' testName=count thr=8 ' make c

c:
#	export JEMALLOC_PATH=$HOME/Downloads/jemalloc-4.1.0
#	MALLOC_CONF=“prof:false”
#	LD_PRELOAD=${JEMALLOC_PATH}/lib/libjemalloc.so.2  ~/nb/benchmarkJava/src/benchmarkJava/benchmark ${ARG}
	~/nb/benchmarkJava/src/benchmarkJava/benchmark ${ARG}
