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


## todo
- 분석을 더해서 perlmod에서도 해당 넣어둔 값들을 이용할수 있게 함수별로 뽑을수 있어야 하고 , perlmod와 xml에서 어떻게 값을 뽑을지를 구상해야 한다.
	- 일단 소스를 보면서 어디에서 어떤 값을 얻을수 있는지를 더 살펴보아야 한다.
	- 아직 정확히 codify 함수들이 수행되는지 모른다.
- 어떻게 class 이름과 함수 이름을 얻을수 있는가? 왜냐면, 해당 함수 이름 밑에 컨디션을 달아두어야 한다.
	- pushScope(g_curClassName);
	- generateFunctionLink(*g_code,yytext);
