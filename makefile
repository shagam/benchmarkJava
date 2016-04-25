

# ARG=' tree size=10000 thr=8 ' make java
# ARG=' noLock size=10000 thr=8 ' make java
# ARG=' prime size=10000 thr=8  ' make java
# ARG=' alloc size=10000 thr=8 ' make java
# ARG=' queue size=10000 thr=8 ' make java
# ARG=' copy  size=10000 thr=8 ' make java
# ARG=' hash  size=10000 thr=8 ' make java

java:
	java -ea  -verbose:gc  -Xloggc:/var/js/java.log_benchmark -jar dist/benchmarkJava.jar ${ARG}

#java -ea -XX:+UseConcMarkSweepGC -XX:+UseParNewGC  -XX:+PrintTenuringDistribution -verbose:gc  -Xloggc:/var/js/java.log_benchmark -jar dist/benchmarkJava.jar ${ARG}

# ARG=' tree size=10000 thr=8 ' make c
# ARG=' noLock size=10000 thr=8 ' make c
# ARG=' prime size=10000 thr=8 ' make c
# ARG=' alloc size=10000 thr=8 ' make c
# ARG=' queue size=10000 thr=8 ' make c
# ARG=' copy  size=10000 thr=8 ' make c

c:
#	export JEMALLOC_PATH=$HOME/Downloads/jemalloc-4.1.0
#	MALLOC_CONF=“prof:false”
	LD_PRELOAD=${JEMALLOC_PATH}/lib/libjemalloc.so.2  ~/nb/benchmarkJava/src/benchmarkJava/benchmark ${ARG}
