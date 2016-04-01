# benchmarkJava
Speed benchmark of java and c for multithread application

###abstract

* speed benchmarks for multithreading programs written in Java and c
* Tests: Tree-updates, alloc, mem-copy

###usage

* java -jar benchmarkJava.jar
* java -jar benchmarkJava.jar   [threads=32] 

c:

* make
* benchmark
* benchmark [threads=32] 

When running with no parameters: uses default parameters.


###arguments

Unique prefix of argument name is enough (No need to type full name).

* threads     - number of concurrent threads</li>
* iterations - loops</li>

###help

* When running with no parameters (Or with) help is printed.
* The help is created automatically.
* See Args.java args.c


### files


* Makefile -  compiles c program</li>
* benchmarkJava.java</li>
* Args.java             Argumet parser for Java (MIT license)</li>
* benchmarkJava.jar
* benchmark.c
* args.c                Argumet parser for c (MIT license)</li>
* args.h


### License

* MIT License, free to change and redistribute, Just keep the credit.
* Any question or requests are welcome

<h3>
<a id="authors-and-contributors" class="anchor" href="#authors-and-contributors" aria-hidden="true"><span aria-hidden="true" class="octicon octicon-link"></span></a>Authors and Contributors</h3>

<p><a href="mailto:eli.shagam@gmail.com">eli.shagam@gmail.com</a></p>

TechoPhil.com
~                                                                                      
~       
