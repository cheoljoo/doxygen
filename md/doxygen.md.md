Done : Cache , EPS test

# Introduction
- this is doxygen documents.
    - draw the plantuml

# Explanation how to process for FAST plantuml


```puml
@startuml
folder config.xml {
	artifact PLANTUML_RUN_FAST
	        
    folder config.xml [
        artifact xml
        ====
        and style
    ]
}

folder debug.cpp {
	artifact "Debug::Plantuml"
    folder debug.h {
	    artifact "Plantuml=0x4000"
    }
}

config.xml -down-> debug.cpp
debug.cpp -down-> docvisitor.cpp

folder docvisitor.cpp {

    folder writePlantUMLSource {
        folder writePlantUMLSource.old [
            write plantuml file (inline_umlgraph_#.pu)
            ----
            writePlantUMLFile()
            generatePlantUMLOutput()
        ]
        folder writePlantUMLSource.new [
            write plantuml file (inline_umlgraph_#)
            writePlantUMLFile()
            generatePlantUMLOutput()
            ----
            add PUML_Type as arguments
            PlantumlManager::instance()->insert(generateType,puName,format,text);
        ]
        writePlantUMLSource -down-> writePlantUMLSource.old : Original
        writePlantUMLSource -down-> writePlantUMLSource.new : PLANTUML_RUN_FAST
    }

    artifact docbookvisitor.cpp
	artifact htmldocvisitor.cpp
	artifact latexdocvisitor.cpp
    artifact rtfdocvisitor.cpp
    artifact vhdldocgen.cpp
}

database PlantumlManager.DB [
    static PlantumlManager     *m_theInstance;
     QDict< QList<QCString> >       m_pngPlantumlFiles;
     QDict< QList<QCString> >       m_svgPlantumlFiles;
     QDict< QList<QCString> >       m_epsPlantumlFiles;
     QDict< QCString >       m_pngPlantumlContent;
     QDict< QCString >       m_svgPlantumlContent;
     QDict< QCString >       m_epsPlantumlContent;
]

writePlantUMLSource.new -down-> PlantumlManager.DB : PLANTUML_RUN_FAST


folder plantuml.cpp {
    folder generatePlantUMLOutput.old [
        run java for each inline_umlgraph_#.pu
    ]
    folder generatePlantUMLOutput.new [
        No run java for run once at the end of process
        Just add image into list
    ]
    writePlantUMLSource.old -down-> generatePlantUMLOutput.old : original        
    writePlantUMLSource.new -down-> generatePlantUMLOutput.new : PLANTUML_RUN_FAST
}

folder doxygen.cpp {
    folder plantuml.h [
        class PlantumlManager
    ]
}

folder plantuml.cpp {
    folder PlantumlManager_run [
        PlantumlManager::instance()->run();
        - run java once at the last time of doxygen process like drawing dot graph
            - read a inline_umlgraph_cache.pu
            - if exist , reuse it.
            - if not exist , add inline_umlgraph_type.pu
            - save all plantuml into inline_umlgraph_cache_all.pu for caching or reusing next time.
        - When we make inline_umlgraph_type.pu , we divide into 4 files for multi-processing.
            - But , prcessing image count  is less than 8 , we will use only one file.
    ]
}

plantuml.h -down-> writePlantUMLSource : PLANTUML_RUN_FAST

PlantumlManager.DB -down-> PlantumlManager_run : PLANTUML_RUN_FAST



@enduml
```

## cache
- QCString                m_cachePlantumlAllContent;  //  = fileToString      = readInputFile
                            // findScopePattern
                            // QCString.find  contains
- instance()  ->  read cache file 
- writePlantULMSource() ->  compare the text with cache contents
	- if matched , pass.
	- if not matched ,  add it.
	- add text to current plantuml
- run() -> save current plantuml into cache file 

## todo  list
- done 
    - html example
    - cache
    - epstopdf run : change the code like the java run
    - find the bug : what is different between Not_FAST and FAST version?
        - special case : classBnSmsReceiveCallback.eps , classBnSmsSendCallback.eps , classBpSmsReceiveCallback.eps , classBpSmsSendCallback.eps , classISmsReceiveCallback.eps , classISmsSendCallback.eps
            - These files are created in different function.  (not generatePlantUMLOutput() ). So this is not the problem in related to this change.  Finally this is not the problem.
- todo
    - multiple thread : divide into several files before running java. (already know the count)
    - get test result beautifully and briefly.


# Test Result
The example has 5 plantumls. 

|        Test Type          |  Original  |  PLANTUML_RUN_FAST     | Modify One Plantuml      | 
|---------------------------|------------|-------------|-----------------|
| Elapsed Time              | 23.370 sec | 7.349 sec   |  4.652 sec      |
| external tools Time       | 23.020 sec | 6.819 sec   |  4.301 sec      |

# Test Environments (How to test)
- The example has 5 plantumls. 
## Original Configration
- I follow the original path to process plantuml.
- Configuration of Doxyfile
    - OUTPUT_DIRECTORY       =  OUTPUT
    - SOURCE_BROWSER         = YES
    - GENERATE_HTML          = YES
      only set the html
    - GENERATE_LATEX         = NO
- Run
    - $ doxygen -d time

## PLANTUML_RUN_FAST 
- Changed Configuration of Doxyfile
    - PLANTUML_RUN_FAST      = YES


## Modify One Plantuml
- Modify one plantuml in 5 plantuml of example  with the same environment of PLANTUML_RUN_TEST

# How to Reduce the performance
## PLANTUML_RUN_FAST
- Make Just one pu (plantuml) file to run java once
    - Just run once when we generate HTML.
### another test with setting YES to all GENERATE_*
- When we generate several types , we run java several times according the different java arguments.
    - set YES to all GENERATE_*
    - There are 20 plantuml in this example and configuration.  But, it runs java just 4 times.
```text
Executing external command `java -Djava.awt.headless=true -jar "/Users/cheoljoo/bin/plantuml.jar" -o "/Users/cheoljoo/Code/github/doxygen/build/bin/B/OUTPUT/docbook" -charset UTF-8 -tpng "/Users/cheoljoo/Code/github/doxygen/build/bin/B/OUTPUT/docbook/inline_umlgraph_pngdocbook.pu" `
10.019 sec: Running PlantUML on png PlantumlFiles in html
Executing external command `java -Djava.awt.headless=true -jar "/Users/cheoljoo/bin/plantuml.jar" -o "/Users/cheoljoo/Code/github/doxygen/build/bin/B/OUTPUT/html" -charset UTF-8 -tpng "/Users/cheoljoo/Code/github/doxygen/build/bin/B/OUTPUT/html/inline_umlgraph_pnghtml.pu" `
17.906 sec: Running PlantUML on png PlantumlFiles in rtf
Executing external command `java -Djava.awt.headless=true -jar "/Users/cheoljoo/bin/plantuml.jar" -o "/Users/cheoljoo/Code/github/doxygen/build/bin/B/OUTPUT/rtf" -charset UTF-8 -tpng "/Users/cheoljoo/Code/github/doxygen/build/bin/B/OUTPUT/rtf/inline_umlgraph_pngrtf.pu" `
25.842 sec: Running PlantUML on eps PlantumlFiles in latex
Executing external command `java -Djava.awt.headless=true -jar "/Users/cheoljoo/bin/plantuml.jar" -o "/Users/cheoljoo/Code/github/doxygen/build/bin/B/OUTPUT/latex" -charset UTF-8 -teps "/Users/cheoljoo/Code/github/doxygen/build/bin/B/OUTPUT/latex/inline_umlgraph_epslatex.pu" `
```

## Modify One Plantuml
- Cached file name : OUTPUT_DIRECTORY/inline_umlgraph_cache_all.pu
    - inline_umlgraph_cache_all.pu was created when doxygen was finished.
    - inline_umlgraph_cache_all.pu had all plantuml contents. and it will be used to check what is changed. 
- If you want to keep pu files , DOT_CLEANUP = NO
    - inline_umlgraph_<type><dictory>.pu includes changed plantuml. So it can reduce the java processing time.

# Explanation of the code
## old
- generatePlantUMLOutput() makes each plantuml from each @start~enduml with the name as inline_umlgraph_#.pu.

## new (PLANTUML_RUN_FAST)
- It will make a plantuml png at the end of processes of doxygen. So we can reduce the count of java runs.
- class PlantumlManager saves all plantuml information for running java at the end of doxygen and for comparing the plantuml contents as a cache.
    - I use writePlantUMLSource() to get more information (pu name and contents).  I modified writePlantUMLSource() because I do not want to read the file to get the contents in generatePlantUMLOutput().
    I add format argument in  writePlantUMLSource().
    -  The writePlantUMLSource() compares each plantuml contents and filename with cached information (inline_umlgraph_cache_all.pu). Then it will decide whether will  recreate or reuse. 
- Like dot , we make plantuml pictures at the end of doxygen.

## my work 
- 2018.01.01 : https://github.com/doxygen/doxygen/pull/6723

# MultiThreading
- 2019.01.14
- calculate thread number
	- typeThread :  svg , html  etc...
	- basic formula : 
		- typeThread * numThreadInJava <= idealThreadNumber /2 
		- typeThread <= idealThreadNumber / 4 
		- numThreadInJava <= 4
- Constructor : create typeThread
- How to ->wait , ->cleanup of thread
	- QThread::wait(unsigned long time = ULONG_MAX) 
		- time milliseconds has elapsed. If time is ULONG_MAX (the default), then the wait will never timeout (the thread must return from run()). This function will return false if the wait timed out.
- doxgen external command
```text
Running plantuml with JAVA...
Generating PlantUML png Files in html
Executing external command `java -Djava.awt.headless=true -jar "/home/cheoljoo.lee/bin/plantuml.jar" -o "/data01/cheoljoo.lee/code/github/doxygen/build/bin/example/A/OUTPUT/html" -charset UTF-8 -tpng "/data01/cheoljoo.lee/code/github/doxygen/build/bin/example/A/OUTPUT/html/inline_umlgraph_pnghtml.pu" `
Generating PlantUML png Files in rtf
Executing external command `java -Djava.awt.headless=true -jar "/home/cheoljoo.lee/bin/plantuml.jar" -o "/data01/cheoljoo.lee/code/github/doxygen/build/bin/example/A/OUTPUT/rtf" -charset UTF-8 -tpng "/data01/cheoljoo.lee/code/github/doxygen/build/bin/example/A/OUTPUT/rtf/inline_umlgraph_pngrtf.pu" `
Generating PlantUML eps Files in latex
Executing external command `java -Djava.awt.headless=true -jar "/home/cheoljoo.lee/bin/plantuml.jar" -o "/data01/cheoljoo.lee/code/github/doxygen/build/bin/example/A/OUTPUT/latex" -charset UTF-8 -teps "/data01/cheoljoo.lee/code/github/doxygen/build/bin/example/A/OUTPUT/latex/inline_umlgraph_epslatex.pu" `
Executing external command `epstopdf "/data01/cheoljoo.lee/code/github/doxygen/build/bin/example/A/OUTPUT/latex/inline_umlgraph_2.eps" --outfile="/data01/cheoljoo.lee/code/github/doxygen/build/bin/example/A/OUTPUT/latex/inline_umlgraph_2.pdf"`
```

# TODO 
- 2019.01.13
	- multiple thread : QThread and Exec
	- running multi processes for different option for java
	- reduce the length of argument (max 4 or 8)
		- use relative path



getMemberFromSymbol
getMemberByName
- Name으로 class에서 member를 찾울수 있다. 
- 얻어진 함수 이름이 나오는 class가기본이 된다. 여기서 이름을 찾는 것이고, 
- :: 으로 된 경우는 그 앞의 것을 사용하면 되고,
- p.  p-> 으로 되어져있는 것은 찾아야 하는데 , 이때 어디서 찾을지가 문제가 되는 것이다. 
