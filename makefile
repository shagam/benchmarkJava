
help:
	java -ea -jar dist/benchmarkJava.jar help
	echo  nemchmark.c
	~/nb/benchmarkJava/src/benchmarkJava/benchmark help 

# ARG=' help ' make java
# ARG=' testName=tree size=10000 thr=8 ' make java
# ARG=' testName=noLock size=10000 thr=8 ' make java
# ARG=' testName=queue size=10000 thr=8 ' make java
# ARG=' testName=hash  size=10000 thr=8 ' make java
# ARG=' testName=alloc size=1000 thr=8 ' make java
# ARG=' testName=copy  size=10000 thr=8 ' make java
# ARG=' testName=prime size=1000 thr=8  ' make java
# ARG=' testName=count  size=10000 thr=8 ' make java

java:
	java -ea  -verbose:gc  -Xloggc:/var/js/java.log_benchmark -jar dist/benchmarkJava.jar ${ARG}

#java -ea -XX:+UseConcMarkSweepGC -XX:+UseParNewGC  -XX:+PrintTenuringDistribution -verbose:gc  -Xloggc:/var/js/java.log_benchmark -jar dist/benchmarkJava.jar ${ARG}

# ARG=' help ' make c
# ARG=' testName=tree size=10000 thr=8 ' make c
# ARG=' testName=noLock size=10000 thr=8 ' make c
# ARG=' testName=queue size=10000 thr=8 ' make c
# ARG=' testName=hash  size=10000 thr=8 ' make c
# ARG=' testName=alloc size=1000 thr=8 ' make c
# ARG=' testName=copy  size=10000 thr=8 ' make c
# ARG=' testName=prime size=1000 thr=8  ' make c
# ARG=' testName=count  size=10000 thr=8 ' make c

c:
#	export JEMALLOC_PATH=$HOME/Downloads/jemalloc-4.1.0
#	MALLOC_CONF=“prof:false”
#	LD_PRELOAD=${JEMALLOC_PATH}/lib/libjemalloc.so.2  ~/nb/benchmarkJava/src/benchmarkJava/benchmark ${ARG}
	~/nb/benchmarkJava/src/benchmarkJava/benchmark ${ARG}
