# why
- automatic flow chart

# source : doxygen/doxygen
- src/doxygen.cpp  -> parseInput()
	-  /**************************************************************************
   *             Parse source files                                         *
   **************************************************************************/
	- parseFiles() -> parseFile()

- SOURCE_BROWSE = YES
	- code.l is running when you change the variable SOURCE_BROWSE in Doxyfile

- code.l
	- It gives information of keywords about conditional keywords.
	- But, I wanna get the condition information. If need it , I should change the code to get the condition.  See more.
	- if\s*(   ->   go into state "FuncCall"
	- it does not have enough information when we generate XML. (Do not have condition value)

# how to use it
- cd test
- doxygen
    - a.cpp is examaple
- if you want to use your files.
    - only use cpp file.
    - but this cpp file should include your headerfile contents .
``` exmaple.cpp
// header contents
class TT { 
    ...
    A()
};

// cpp contents
TT::A()
{
}
```

# perlmod
- make markdown document in perlmod
- perlmodgen.cpp
- new idea : combine between perlmod markdown and xml keywordflow flow chart and test cases ...
	- we need to analyze the code of xmlgen and perlmodgen.
-  flow :
	- generatePerlMod() -> pmg.generate()->
	- generatePerlModOutput()
		- $doxydocs =
		- iterate class ClassDef :
		- generatePerlModForClass(cd)
			- cd->baseClasses()
				- openList("base") iterator BaseClassDef
			- cd->subClasses()
				- openList("derived") iterator BaseClassDef
			- cd->getClassSDict()
				- openList("inner") ClassSDict::Iterator  ClassDef
			- cd->includeInfo()
				- openHash("includes")
			- addTemplateList(cd,m_output)
				- openList("template_parameters") ArgumentListIterator
			- addListOfAllMembers(cd)  => make a plantuml for each class
				- openList("all_members") MemberNameInfoSDict::Iterator MemberInfo
			- member group
			- generatePerlModSection
				- name
					- public_typedefs
					- public_methods
					- public_members
					- public_slots
					- signals
					- dcop_methods
					- properties
					- public_static_methods
					- public_static_members
					- protected_typedefs
					- protected_methods
					- protected_members
					- protected_slots
					- protected_static_methods
					- protected_static_members
					- private_typedefs
					- private_methods
					- private_members
					- private_slots
					- private_static_methods
					- private_static_members
					- friend_methods
					- related_methods
				- openHash(name)
					- openList("members")
				- MemberListIterator mli(*ml); MemberDef *md;
					- generatePerlModForMember(MemberDef *md, Definition *d);
						- kind,name,virtualness,protection,static,brief,detailed,
						- if (func) -> const,volatile ,parameters
						- addPerlModDocBlock(m_output,"brief",md->getDefFileName(),md->getDefLine(),md->getOuterScope(),md,md->briefDescription())
						- addPerlModDocBlock(m_output,"detailed",md->getDefFileName(),md->getDefLine(),md->getOuterScope(),md,md->documentation())
			- addPerlModDocBlock(m_output,"brief",cd->getDefFileName(),cd->getDefLine(),cd,0,cd->briefDescription())
			- addPerlModDocBlock(m_output,"detailed",cd->getDefFileName(),cd->getDefLine(),cd,0,cd->documentation())
		- generatePerlModForNamespace(NamespaceDef *nd)
		- generatePerlModForFile(FileDef *fd)
		-

# XML
- GENEARATE_XML = YES
- make markdown document in xml
- xmlgen.cpp
- analysis
	- geneateXML();
		- generateXMLForClass(cd,t)
		- generateXMLForNamespace(nd,t)
		- generateXMLForFile(fd,t)   // generate AKlass.cpp // show Keyword flow (for if ... etc)
			- include file lists
			- DotInclDepGraph::writeXML(FTextStream &t)
				- <invincdepgraph>
			- writeInnerClasses(fd->getClassSDict(),t);
			- writeInnerNamespaces(fd->getNamespaceSDict(),t);
				- <innernamespace refid="namespacefamily">family</innernamespace>
			- keywordflow
			```cpp
			if (Config_getBool(XML_PROGRAMLISTING))
			1633   {
			1634     t << "    <programlisting>" << endl;
			1635     writeXMLCodeBlock(t,fd);
			1636     t << "    </programlisting>" << endl;
			1637   }
			```
			- writeXMLCodeBlock(t,fd)
				```cpp
				g_lang2extMap[] =
				{
				//  language       parser           parser option
				{ "idl",         "c",             SrcLangExt_IDL      },
				{ "java",        "c",             SrcLangExt_Java     },
				{ "javascript",  "c",             SrcLangExt_JS       },
				{ "csharp",      "c",             SrcLangExt_CSharp   },
				{ "d",           "c",             SrcLangExt_D        },
				{ "php",         "c",             SrcLangExt_PHP      },
				{ "objective-c", "c",             SrcLangExt_ObjC     },
				{ "c",           "c",             SrcLangExt_Cpp      },
				{ "c++",         "c",             SrcLangExt_Cpp      },
				{ "slice",       "c",             SrcLangExt_Slice    },
				{ "python",      "python",        SrcLangExt_Python   },
				{ "fortran",     "fortran",       SrcLangExt_Fortran  },
				{ "fortranfree", "fortranfree",   SrcLangExt_Fortran  },
				{ "fortranfixed", "fortranfixed", SrcLangExt_Fortran  },
				{ "vhdl",        "vhdl",          SrcLangExt_VHDL     },
				{ "xml",         "xml",           SrcLangExt_XML      },
				{ "sql",         "sql",           SrcLangExt_SQL      },
				{ "tcl",         "tcl",           SrcLangExt_Tcl      },
				{ "md",          "md",            SrcLangExt_Markdown },
				{ 0,             0,              (SrcLangExt)0        }
				};
				```
				```bison
				<Body>{FLOWKW}/{BN}*"(" 			{
									startFontClass("keywordflow");
									codifyLines(yytext);
									endFontClass();
									g_name.resize(0);g_type.resize(0);
									g_inForEachExpression = (qstrcmp(yytext,"for each")==0 || qstrcmp(yytext, "foreach")==0);
									BEGIN(FuncCall);
									}
				<Body>{FLOWCONDITION}/{BN}*"("          {
									if (g_currentMemberDef && g_currentMemberDef->isFunction())
									{
									g_currentMemberDef->addFlowKeyWord();
									}
									startFontClass("keywordflow");
									codifyLines(yytext);
									endFontClass();
									g_name.resize(0);g_type.resize(0);
									g_inForEachExpression = (strcmp(yytext,"for each")==0 || strcmp(yytext, "foreach")==0);
									BEGIN(FuncCall);
									}
				```
				- codifyLines -> codify
				- if 관련은 color를 이용하여  syntax를 보이는 것은 문제가 없으나, 실제로 condition이 무엇인지를 찾는 것은 다른 일이 된다.
					- 그러므로 , condition은 따로 계산하도록 해야 할 것으로 보인다. 일단 코드 라인 기준으로 작업해야 할지?


# code.l
- 모든 곳에 print를 넣어  기본 if / while / 등에서 나오는 것에 대해 어떤 률을 거치는지는 찾아냄.
- { 의 처음은 FLOWKEYWORD를 찾는 것으로 대치가 되며 , 그 이후에 { } 은 순서대로 나오므로 count를 하면 된다.
- " " 에 대해서는 skip.. 으로 넘어가서 나오지가 않는 것이다.
- 결국 필요한 것에 대해서 ( ) 의 수와 { } 의 수를 통한 depth를 check하면 flow chart를 그릴수 있는 내용은 뽑아낼수 있다.
- 그러므로 , code.l 에서 나오는 것을 어떤 구조체에 넣어서 관리를 해야 할지 설계를 추가해야 한다.
	- 일반적으로 함수 이름 밑으로 들어가게 하면 될 것으로  보인다.
	- class이면 class 및의 member 변수로 들어갈 것이고,
	- C 부분이면 일반적인 붑분의 구조체에 걸리게 될 것으로 보인다.
- 일단 BROWSER 나 XML을 모두 off 시켜도 code.l 은 수행이 되더라. 확인했음.

## 190820 code.l
- we can get more information in FLOWCONDITION start status
    ```cpp
		<Body>{FLOWCONDITION}/{BN}*"("          {
                                        printf("<Body FLOWCONDITION/BN %s\n",yytext);
                                          if (g_currentMemberDef && g_currentMemberDef->isFunction())
                                          {
                                            g_currentMemberDef->addFlowKeyWord();
                                            printf("<Body FLOWCONDITION/BN declaration=%s\n",g_currentMemberDef->declaration());
                                            printf("<Body FLOWCONDITION/BN definition=%s\n",g_currentMemberDef->definition());
                                            printf("<Body FLOWCONDITION/BN typeString=%s\n",g_currentMemberDef->typeString());
                                            printf("<Body FLOWCONDITION/BN argsString=%s\n",g_currentMemberDef->argsString());
                                            printf("<Body FLOWCONDITION/BN numberOfFlowKeyWords=%d\n",g_currentMemberDef->numberOfFlowKeyWords());
                                            FileDef *tfd = g_currentMemberDef->getFileDef();
                                            printf("<Body FLOWCONDITION/BN file name=%s lineNr=%d\n",tfd->name().data(), g_yyLineNr);
                                            printf("<Body FLOWCONDITION/BN file getSourceFileBase=%s\n",tfd->getSourceFileBase().data());
                                            printf("<Body FLOWCONDITION/BN file getPath=%s\n",tfd->getPath().data());
                                            printf("<Body FLOWCONDITION/BN file title=%s\n",tfd->title().data());
                                            // g_currentMemberDef->memberDefinition());
                                          }

    ```
- general context for flow control
	```cpp
		do {
				break;
		} while(condition);
		
		if (condition) ...;

		if (condition){
		} else if (condition2){
		} else {
			if(condition3) {   }
		}

		switch(variable){
			case a:
			break;
			case b:
			break;
			default:
			break;
		};

		for( A ; B ; C) {
		}
	```
    - FLOWKW / FLOWCONDITION
    	```bison
	    FLOWKW ("break"|"catch"|"continue"|"default"|"do"|"else"|"finally"|"return"|"switch"|"throw"|"throws"|"@catch"|"@finally")
    	FLOWCONDITION  ("case"|"for"|"foreach"|"for each"|"goto"|"if"|"try"|"while"|"@try")
    	```
        -  goto => show rectangle  / no action
        - try .. catch => no action
        - break => show rectangle / no action

- programming design for flow keywords & conditions
    - vector<FlowInfo> 로부터 그림과 정보를 뽑아낼수 있음.
    - struct FlowInfo { QString flow; QString condition; int depth; int lineNr; QString filename;};
    - depth를 보고 []을 증가하며 , depth가 더 커질때는 recursive하게 처리
    - plantuml로 그림을 그려준다.  perl로도 C++로도 그림을 그려줄수 있다.  각기 개별 file로 만들어주는게 좋을 듯.
    - 결과는 그려진 내용을 !INCLUDE해서 사용하면 제일 좋을 듯 하다. 
    	- 어떤 과정에서 개별 파일로 만들어줄 것인가?
        - code.l : add information in vector<FlowInfo> of each member
        - perlmod.cpp : 
            - First : add plantuml in perl output after generating plantuml with vector<FlowInfo>
              we can reuse this code when we make markdown-plantuml
            - Alternatives : add vector<FlowInfo> in perl output 
        - CGADoxygen : generate each plantuml file and put this result in SDD
    - plantuml comments
        - comments of plantuml
            - ```
                @startuml
                ' This is a comment on a single line
                Bob->Alice : hello
                /' You quote alors use slash-and-quote
                to split your comments on several
                lines '/
                @enduml
                ```
            - ' @CGA_VARIANT를 이용하여 같은 방식으로 merge수행


# bison (lex) - 2019.08.13
- doxygen.cpp
	- initDoxygen()
		- ```cpp
			Doxygen::parserManager = new ParserManager;
  			Doxygen::parserManager->registerDefaultParser(         new FileParser);
  			Doxygen::parserManager->registerParser("c",            new CLanguageScanner);
			```
		- ```Doxygen::parserManager->getParser(extension)``` -> parseCode()
- perlmodgen.cpp
	- parseCode(m_ci,s->context(),s->text(),FALSE,0);
- pre.l (doxygen.cpp -> preprocessFile() )
	- 기본적으로 pre-processing (include/ pragma / MACARO /#ifdef / @cond / @verbatim 처리)을 하는 것으로 if (...)  과 같은 것을 한번에 해결하고 , g_culyCount 를 이용하여 depth도 유추할수 있지만,
	- 미진한 부분
		- else / else if 에 대한 처리 없음.
		- for(a=3;  \n  ...) 과 같이 여러줄로 처리하는 부분에 대해서는 처리 불능
	- global variables
		- static QDict<void> g_allIncludes(10009);
		- static QStack<CondCtx>    g_condStack; // @cond
	- 사용 함수
		- unput() : 처리하지 않은 것 처럼 다시 string에 넣어 한번 더 처리하도록
		- preYYlex() : yylex()와 같은 것으로 lex를 수행시켜주는 것이다.  보통 file이름 . YYlex()  라는 이름으로 생성된다.
	- preFreeScanner() -> preYYlex_destroy()  : 끝낼때 free해주는 것이다.
- scanner.l (C-like language parser) -> CLanguageScanner class
	- parseMain -> scannerYYlex
		- class Entry : parseMain() in scanner.l will generate a tree of these entries. 
		- 
	- parseCode -> ::parseCCode (code.l) 호출
	- scanFreeScanner() -> scannerYYlex_destroy()  :  끝낼때 free해주는 것이다.
	- New Learning
		- ```<FindMembers,FindMemberName>{SCOPENAME} {  if (YY_START==FindMembers) ``` 현재 state check
		- ```static int              lastSkipRoundContext;   lastSkipRoundContext = FindMembers;  BEGIN( lastSkipRoundContext) ;```  저장해두고 사용하기도 한다.
- scanner.l이 끝난후에 Building .. 을 하는 것이 있다. (doxygen.cpp)
- doxygen.cpp :: parseInput()
	- Building group list...
		- ```buildGroupListFiltered(EntryNav *rootNav,bool additional, bool includeExternal)```  
			- GroupDef *gd = Doxygen::groupSDict->find(root->name);
			- gd = new GroupDef(root->fileName,root->startLine,root->name,root->type,rootNav->tagInfo()->fileName);
	- Building directory list...
		- buildDirectories()  : dirdef.cpp
			- dir=Doxygen::directories->find(fd->getPath())
			- ```cpp
				DirDef *dir;
				DirSDict::Iterator sdi(*Doxygen::directories);
				for (sdi.toFirst();(dir=sdi.current());++sdi)
				```
			- Doxygen::directories->sort();
  			- computeCommonDirPrefix();
		- findDirDocumentation(rootNav);
			- DirDef *dir;
	- Building namespace list...
	- Building file list...
		-  buildFileList(EntryNav *rootNav)
			- FileDef *fd=findFileDef(Doxygen::inputNameDict,root->name,ambig);
	- Building class list...
		- buildClassList(rootNav);
			- addClassToContext(rootNav);
				- cd=new ClassDef(tagInfo?tagName:root->fileName,root->startLine,root->startColumn,
				- Doxygen::classSDict->append(fullName,cd);
				- Doxygen::genericsDict->insert(fullName,cd);
	- Building example list...
		- buildExampleList(rootNav);
			- Doxygen::exampleSDict->find(root->name)
			- Doxygen::exampleSDict->inSort(root->name,pd)
	- earching for enumerations...
		- findEnums(rootNav);
			- if ((cd=getClass(scope))==0) nd=getResolvedNamespace(scope)
			- md = new MemberDef( root->fileName,root->startLine,root->startColumn,0,name,0,0,root->protection,Normal,FALSE,
			- md->setEnumBaseType(root->args);
			- md->setFileDef(fd);
			- MemberName *mn; if ((mn=(*mnsd)[name]))
	- Searching for documented typedefs...
	- Searching for members imported via using declarations...
	- Searching for included using directives...
	- Searching for documented variables...
	- Building interface member list...  // UNO IDL
	- Building member list...   // using class info only !
		- buildFunctionList(EntryNav *rootNav)
			- addMethodToClass(rootNav,cd,rname,isFriend);
			-  MemberName *mn; MemberDef *md=0; if ((mn=Doxygen::functionNameSDict->find(rname)))
			- gd = Doxygen::groupSDict->find(root->groups->getFirst()->groupname.data());
			- addMemberToGroups(root,md);
	- Searching for friends...
	- Searching for documented defines...
	- Computing class inheritance relations...
	- Computing class usage relations...
	- Flushing cached template relations that have become invalid...
	- Computing class relations...
	- Add enum values to enums...
	- Searching for member function documentation...
	- Creating members for template instances...
	- Building page list...
	- Search for main page...
	- Computing page relations...
	- Determining the scope of groups...
	- Sorting lists...
        - Doxygen::memberNameSDict->sort();
        - Doxygen::functionNameSDict->sort();
        - Doxygen::hiddenClasses->sort();
        - Doxygen::classSDict->sort();
	- Freeing entry tree
	- Determining which enums are documented
	- Computing member relations...
	- Building full member lists recursively...
        - buildCompleteMemberLists()
            - ClassDef *cd;
            - ClassSDict::Iterator cli(*Doxygen::classSDict);
            - for (cli.toFirst();(cd=cli.current());++cli)
            - if (cd->memberNameInfoSDict()) cd->memberNameInfoSDict()->sort();
	- Adding members to member groups.
	- Computing member references...
	- Inheriting documentation...
	- Generating disk names...
	- Adding source references...
	- Adding xrefitems...
	- Sorting member lists...
	- Computing dependencies between directories...
	- Generating citations page...
	- Counting data structures...
	- Resolving user defined references...
	- Finding anchors and sections in the documentation...
	- Transferring function references...
	- Combining using relations...
	- Adding members to index pages...
- doxygen.cpp :: generateOutput()
    - ```cpp
        g_outputList = new OutputList(TRUE);
        g_outputList->add(new HtmlGenerator);
            HtmlGenerator::init();
        ```
	- Generating style sheet...
        - g_outputList->writeStyleInfo(0); // write first part
            - ```cpp
                #define FORALL1(a1,p1)                                        \
                void OutputList::forall(void (OutputGenerator::*func)(a1),a1) \
                {                                                             \
                  QListIterator<OutputGenerator> it(m_outputs);               \
                  OutputGenerator *og;                                        \
                      for (it.toFirst();(og=it.current());++it)               \
                      {                                                       \
                        if (og->isEnabled()) (og->*func)(p1);                 \
                      }                                                       \
                }
                FORALL1(int a1,a1)
                ```
            - forall(&OutputGenerator::writeStyleInfo,part);
	- Generating search indices...
	- Generating example documentation...
	- Generating file sources...
        - Parsing code for file a.cpp...
		- generateFileSources();
			- ```cpp
			      FileNameListIterator fnli(*Doxygen::inputNameList);
			      FileName *fn;
			      for (;(fn=fnli.current());++fnli)
			      {
			        FileNameIterator fni(*fn);
			        FileDef *fd;
			        for (;(fd=fni.current());++fni)
			        {
			          QStrList filesInSameTu;
			          fd->startParsing();
			          if (fd->generateSourceFile() && !g_useOutputTemplate) // sources need to be shown in the output
			          {
			            msg("Generating code for file %s...\n",fd->docName().data());
			            fd->writeSource(*g_outputList,FALSE,filesInSameTu);
			
			          }
			          else if (!fd->isReference() && Doxygen::parseSourcesNeeded)
			            // we needed to parse the sources even if we do not show them
			          {
			            msg("Parsing code for file %s...\n",fd->docName().data());
			            fd->parseSource(FALSE,filesInSameTu);
			          }
			          fd->finishParsing();
			        }
			      }
                ```
            - fd->parseSource(FALSE,filesInSameTu); -> FileDef::parseSource(bool sameTu,QStrList &filesInSameTu) -> pIntf->parseCode() -> parseCCode()
	            - parseCode -> ::parseCCode (code.l) 호출
	- Generating file documentation...
	    - Generating docs for file a.cpp...
            - fd->writeDocumentation(*g_outputList); // filedef.cpp
	    - Generating caller graph for function example : 
	    - Generating call graph for function main
            - FileDef::writeDocumentation(OutputList &ol) -> writeMemberDocumentation(ol,lmd->type,lmd->title(lang)); -> ml->writeDocumentation(ol,name(),this,title);
            - MemberDef::writeDocumentation -> MemberDef::_writeCallerGraph(OutputList &ol)
	- Generating page documentation...
	- Generating group documentation...
	- Generating class documentation...
	    - Generating docs for compound ClassA...
            - generateClassList(ClassSDict &classSDict) : generateClassList(*Doxygen::classSDict);
                - cd->writeDocumentation(*g_outputList);
                    - ClassDef::writeDocumentation(OutputList &ol)
                - cd->writeMemberList(*g_outputList);       // only for html
                - cd->writeDocumentationForInnerClasses(*g_outputList);
	    - Generating docs for compound DerivedA...
	- Generating namespace index...
	- Generating graph info page...
	- Generating directory documentation...
	- Generating index page...
	- Generating page index...
	- Generating module index...
	- Generating namespace index...
	- Generating namespace member index...
	- Generating annotated compound index...
	- Generating alphabetical compound index...
	- Generating hierarchical class index...
	- Generating graphical class hierarchy...
	- Generating member index...
	- Generating file index...
	- Generating file member index...
	- Generating example index...
	- finalizing index lists...
	- writing tag file...
	- Generating Perl module output...
        - ```cpp
        if (Config_getBool(GENERATE_PERLMOD))
        {
            g_s.begin("Generating Perl module output...\n");
            generatePerlMod();  // perlmodgen.cpp
            g_s.end();
        }
        void generatePerlMod()
        {
            PerlModGenerator pmg(Config_getBool(PERLMOD_PRETTY));
            pmg.generate();
        }
            ```
	    - Running plantuml with JAVA...- 

# markdown plantuml
- branch : generate_markdown
- 


# branches
- input example
    mytools/input/branch_coverage_for_flow_chart/a.cpp
- analysis_of_man 
    - 분석을 하는 것으로 , 여기서는 log를 넣어서 다 찍어보면서 분석을 하는 것이다.   분석하기위한 printf 문들이 주를 이룬다.
    - 위의 분석이 이를 기반으로 이루어졌다.
    - doxygen.cpp -> pre.l -> scanner.l -> code.l
    - 주로 code.l의 각 state에 대한 log를 삽입하였다.
- 190820/flowKeywordsConditions <- analysis_of_man
    - flow chart를 그릴 수 있는 정보를 얻기 위한 작업을 한 것이다. analysis_of_man의 분석에 더해서 작업을 하였다. (log가 모두 남겨있음)
    - define MemberFlowInfo class to gether flow control information in Memberdef class.
        - code.l : test code for QList to put Flow information
        - memberdef.h : add MemberFlowInfo class to gather flow control information
        - memberdef.cpp : add member functions to insert the node  :addFlowInfo()
    - flow control / keyword parsing is completed. make related functions.
        - code.l :
            - else if
            - gather flow information in each member function
            - parsing is ok. but m_flowinfo is not proper.
        - memberdef.h : define flow keyword functions
        - memberdef.cpp : complete the Flow keyword functions
    - else if /  find condition of flow keywords  / QList control
        - code.l
            - set the proper value for MemberFlowInfo : filename etc..
            - m_hasCondition
            - control whether it is in flow control or not with g_currentMemberFlowInfoBool
            - check "else if"  with g_currentMemberFlowInfoElse
            - find condition of flow keywords
        - memberdef.cpp
            - add void MemberDef::addFlowInfo(MemberFlowInfo mfi)
            - QList control : print and traverse and append properly
        - memberdef.h
            - add definition
    - process switch / case / default in switch statement
        - code.l
            - g_currentMemberFlowInfoCase : case has different type of condition.  ex)  case condition :
        - memberdef.cpp
            - add Default
        - memberdef.h
            - add Default
    - You can find the final result each flow statements with [ADDFLOW]
- 190830/flowKeywordsMakeFileDirectly <- 190820/flowKeywordsConditions
    - issue : QFile & QStreamText 일때 class QStreamText;라고 하여 전방에 선언되어져잇으면 error가 발생한다. 
        - error: variable ‘QTextStream t’ has initializer but incomplete type
        - <qstreamtext.h> 를 추가해 주어야 한다. 이유는 전방선언으로 초기화는 되었지만, class 선언이 없어 member들의 제대로 된 정보가 없는 것이다. refer to doxygen.cpp
    - write the plantuml in addFlowInfo() of memberdef.cpp
    - this is temporary project to show the result for reuse fast.
    - I will reuse this code  in branch : ???/flowPerlmodPlantuml
    - input data : mytools/input/branch_coverage_for_flow_chart/a.cpp
    - refer to [Analysis report](./doxygen_parse_190830_flowKeywordsMakeFileDirectly.md)
    - weak point : solution - use the brace "{  }"
        - for(;;) if (aa) a=1;   ->  for(;;){ if(aa){ a=1; } }
- 190824/flowPerlmod <- 190820/flowKeywordsConditions
    - perlmod를 분석해서 이 안에 위의 MemberFlowInfo를 추가해야 할 것이다.  문제는 없지만, structure 까지 고려를 해야 한다.  그리고, 필요하면 그 안에서 call 하는 것도 넣을수 있을 것이다. 
    - PerlModGenerator::generatePerlModOutput()
        - m_output.add($doxydocs=) . openHash('{',);
            -      $doxydocs=        `\n.indent:0` { __increaseIndent__ __m_blockStart=true__
        - m_output.openList("classes");  -> iopen('[',"classes");
            - `\n.indent:1` classes => [ __increaseIndent__ __m_blockStart=true__
            - generatePerlModForClass(ClassDef *cd)
                - base / sub class name
                - include
                - template
                - all members name and kind
                - generatePerlModSection (memberGroup->members) : user_defined section 
                    - Common Doxygen Page Tags
                    - @page @section
                        - This tag tells Doxygen that this is a free floating page and allows doxygen to name the page so that otehr pages can reference and link to the page. The first word after @page is the word that will need to be type in a @ref command to link the page.
                        - It is often useful to divide a doxygen page into sections and subsections. The serves two purposes. One it creates headers. Second reference can be created to thorugh the page to link to the section. This page makes of sections and links to sections. The list of section at the beginning of the page is implemented using the @section command. Similar to the @page command, a link name and string can be provided.
                - generatePerlModSection( cd->getMemberList(MemberListType_pubMethods) ) : member public / private members
                    - all types of members : methods , members , slots , signals , properties , static , typedefs , friends , related
                    - name => { members => [ 
                        - generatePerlModForMember(md,d);  // each member : for (mli.toFirst();(md=mli.current());++mli) generatePerlModForMember(md,d); 
                            - kind , name , virtualness , protection , static , type
                            - addPerlModDocBlock(m_output,"brief",md->getDefFileName(),md->getDefLine(),md->getOuterScope(),md,md->briefDescription());
                            - addPerlModDocBlock(m_output,"detailed",md->getDefFileName(),md->getDefLine(),md->getOuterScope(),md,md->documentation());
                                - ```cpp    
                                        DocNode *root = validatingParseDoc(fileName,lineNr,scope,md,stext,FALSE,0);
                                        output.openHash(name);
                                        PerlModDocVisitor *visitor = new PerlModDocVisitor(output);
                                        root->accept(visitor);
                                        visitor->finish();
                                        output.closeHash();
                                        delete visitor;  delete root;
                                    ```
                                - log를 넣었는데 ,  detailed / brief일때 뒤에 link등을 달수 있게 하는 것이 포함되어져있다.  perl에서도 html 처럼 link를 달수 있게 하는 것을 뽑아둔 것으로 보인다.
                                - visitor의 각각에는 DocLinkedWord 같은 것에서는 url type으로 해서 anchor와 word 를 달아주게 되어진다.
                                - validating을 한후에 그 정보를 기반으로 visitor의 내용들로 변환이 되면서 처리가 된다. 
                                - perlmodgen 을 가지고 그대로 markdown에 적용하면 될 것으로 보인다.  예제가 부족할 뿐이다.
                                - accept() 를 validatiingParseDoc()을 수행한후에 수행시켜주며   
                                    - visitPre , visit , visitPost 를 인자의 타입에 따라서 수행하도록 설계가 되어져있다.
                                - type
                                    - DocWord 
                                    - DocWhiteSpace 
                                    - DocLinkedWord   :  ex. example()  main()
                                    - CompAccept<DocPara> :    @param 
                                        - @param , @retval ->  CompAccept<DocPara>::accept , CompAccept<DocParamSect>::accept , DocParamList::accept m_type m_dir, CompAccept<DocPara>::accept
                            - if func : 
                                - const , volatile
                                - parameters : List
                                    - declaration, type , array , default_value , attributes
                            - define : maybe I do not know it is needed for markdown
                                - parameters : List
                                    - name => ...   : Hash
                            - initializer 
                            - exception
                            - enum
                                - values : List
                                    - name => ... : Hash
                                    - initializer =>  ... 
                                    - brief 
                                    - detailed
                            - reimplemented
                            - reimplemented_by
                - brief for class
                - detailed for class
            - namespace : generatePerlModForNamespace(nd);
            - files : generatePerlModForFile(fd);
            - groups : generatePerlModForGroup(gd);
            - pages : generatePerlModForPage(pd);
            - main page
        - m_output.closeList();  -> iclose(']');
            - __decreaseIndent__ `\n.indent:0` ] __m_blockStart=false__

 


# todo
- Done 모두 : 190820/flowKeywordsConditions branch
    - 분석을 더해서 perlmod에서도 해당 넣어둔 값들을 이용할수 있게 함수별로 뽑을수 있어야 하고 , perlmod와 xml에서 어떻게 값을 뽑을지를 구상해야 한다.
	    - 일단 소스를 보면서 어디에서 어떤 값을 얻을수 있는지를 더 살펴보아야 한다.
	    - 아직 정확히 codify 함수들이 수행되는지 모른다.
    - 어떻게 class 이름과 함수 이름을 얻을수 있는가? 왜냐면, 해당 함수 이름 밑에 컨디션을 달아두어야 한다.
 	    - pushScope(g_curClassName);
	    - generateFunctionLink(*g_code,yytext);
    - 실제로 위의 방법처럼 처리하지는 않았음. 아래의 190820/flowKeywordsConditions branch 설명 을 보기 바란다. 
- 190820/flowKeywordsConditions 이후
    - Done : 190830/flowKeywordsMakeFileDirectly <- 190820/flowKeywordsConditions
        - plantuml.md is the result with flow charts from source code for each function
        - Todo : call function을 추가 / 각각에 partition으로 추가하는 부분을 추가
        - Todo : sequence diagram 을 그릴수 있기도하겠지만, 이것은 전체를 기반으로 만들어야해서 시간이 무지 많이 들 것이다.
            - class 단위로 주고 받는 것에 대해서 각 함수별로 sequence diagram을 그릴수 있을 것이다. 
    - perlmod의 결과로 이 값들이 나오게 한다.   (branch : 190824/flowPerlmod)
    - perlmod안에서 plantuml로도 결과가 나오게하는 C++ Code도 같이 작성한다.  (branch : ???/flowPerlmodPlantuml)
- 190830/flowKeywordsMakeFileDirectly <- 190820/flowKeywordsConditions
    - doc 와는 다르게 flow를 추가를 해야 할 것이다.
        - DoxyStructure.pm 도 맞춰주어야 하나 , 이미 내용이 맞지 않는다. default_value같은 것도 들어있지 않다. 
        - 나의 doxy2cga.pl을 넣어서 python 값까지 얻을수 있게 해주는게 더 좋을 것으로 보인다. 
- 190824/flowPerlmod  (여기서는 분석을 완료하는 것 까지만이다)
    - visitor 인식 
    - 뭔가 기능을 추가하는것은 다른 branch에서 처리
    - 이렇게 분석이 완료되면 뒤로 돌아가서 flow structure 기반으로 function 을 추가해야 할 것이다.
        - 함수가 있으면 그 함수 이름만이라도 써 넣는 것이다.  그러면 조금 더 이해하기 쉬워질 것이다.
        - 또 다른 방법 으로  ,  doxygen comments를 이용할수 있는 방안이 있을까 행각했지만 거기에 doxygen을 넣는 것은 doxygen에 기본적으로 처리를 하지 않기에 어떻게 할 방법이 없다.   그냥 뒤에 들어간 function 이름이 있으면 그 정도만 처리하는 것이 최선일 것으로 보인다.
    - markdown에서는 visitor의 기능이 필요할 수 있다. 이유는 param 처리가 이 안으로 들어가 있는 것으로 보인다. . 각 token 들을 찾아서 link등을 달아주는 것인데 , 실제로 markdown에서는 그렇게 까지 만들 필요가 없기 때문이다.
        - markdown을 위해서는 perlmod안의 visit 내용까지 그대로 모두 살려주어야 detailed doc 안의 값을 모두 살려 제대로 된 문서를 만들수 있다. 
        - plantuml.jar의 위치가 올바르지 않으면 plantuml 을 아예 파싱을 하지 않는다. 
- 190911/flowAddFunctionInBranch <- 190820/flowKeywordsConditions , 190824/flowPerlmod , 190830/flowKeywordsMakeFileDirectly
    - call function들까지 보여줄수 있는지를 본다.  해당 함수에 대해서 어느 class의 함수를 call 했는지까지 보여준다. 
        - 이렇게 되면 sequence diagram까지도 그릴수 있을 것이다.  
- 190916/flowSequenceDiagram <- 190911/flowAddFunctionInBranch <- 190820/flowKeywordsConditions , 190824/flowPerlmod , 190830/flowKeywordsMakeFileDirectly
    - make sequence diagram in plantuml.md
    - _C_ is for C code
        - If you need to separate  C function and binutils funciton, , add more code.
- FlowChart_SequenceDiagram <- 190916/flowSequenceDiagram <- 190911/flowAddFunctionInBranch <- 190820/flowKeywordsConditions , 190824/flowPerlmod , 190830/flowKeywordsMakeFileDirectly
    - release branch of cheoljoo/doxygen : FlowChart_SequenceDiagram
- 190918/flowPerlmod <- FlowChart_SequenceDiagram <- 190916/flowSequenceDiagram <- 190911/flowAddFunctionInBranch <- 190820/flowKeywordsConditions , 190824/flowPerlmod , 190830/flowKeywordsMakeFileDirectly
    - add class MemberFlowInfo into DoxyDocs.pm
        - array : List<MemberFlowInfo> m_flowInfo
        - flowChartPlantuml : QCString m_flowChartPlantuml
        - sequenceDiagramPlantuml : QCString m_sequenceDiagramPlantuml
    - doxy2cga.pl -> default.GV

- original doyxgen에 넣어야 할 것
    - add flow class into memberDef class
    - add flow values into perlmod
    - generate markdown from perlmod
        - add class diagrm in markdown
        - add flow activity in markdown
        - inner
        - show overview of class diagram
    - if need , draw the sequence diagram : investigate caller/callee in doxygen
        - caller and callee for each methods
- What do I find new function
    - caller / callee
        - this is for total drawing of sequence diagram
    - how to find the class from the methods name
        - to draw sequence diagram automatically

