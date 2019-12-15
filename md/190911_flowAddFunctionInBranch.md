# branch : 190911/flowAddFunctionInBranch
- get call function name
    - ```Body>{SCOPETNAME}/{BN}*"("     { // a() or c::a() or t<A,B>::a() or A\B\foo()```
    - ```printf("<Body SCOPENAME/B3 %s\n",yytext);```
        - this is call function in any funciton.
```
+printf("<Body line=%d SCOPENAME/B3 %s\n",g_yyLineNr,yytext);
                                          addType();
                                          generateFunctionLink(*g_code,yytext);
                                          g_bracketCount=0;
                                          g_args.resize(0);
                                          g_name+=yytext;
+                      {
+                          printf("A\n");
+                          MemberFlowInfo tempMemberFlowInfo;
+                          tempMemberFlowInfo.m_flow = MemberFlowInfo::CallFunction;
+                          tempMemberFlowInfo.m_hasCondition = TRUE;
+                          tempMemberFlowInfo.m_condition = yytext;
+                          printf("A1\n");
+                          tempMemberFlowInfo.m_depth = g_bodyCurlyCount;
+                          if(g_currentMemberDef){
+                            FileDef *tfd = g_currentMemberDef->getFileDef();
+                            printf("A2\n");
+                            tempMemberFlowInfo.m_filename = tfd->name();
+                            printf("A3\n");
+                            tempMemberFlowInfo.m_lineNr = g_yyLineNr;
+                            printf("A4\n");
+                            g_currentMemberDef->addFlowInfo(tempMemberFlowInfo);
+                            printf("B\n");
+                          }
+                      }
```

- result
```
[ADDFLOW] func:int example(int &aa, int &bb) flow:functioncall has:1 depth:2 f:a.cpp l:67 cond:ClassA::classAfunc
[0] flow:functioncall has:1 depth:0 f:a.cpp l:60 cond:example
[1] flow:if has:1 depth:1 f:a.cpp l:62 cond: ( aa < bb )
[2] flow:else has:0 depth:1 f:a.cpp l:65 cond:
[3] flow:functioncall has:1 depth:2 f:a.cpp l:67 cond:ClassA::classAfunc

[ADDFLOW] func:int main() flow:return has:0 depth:1 f:a.cpp l:193 cond:
[0] flow:functioncall has:1 depth:0 f:a.cpp l:73 cond:main
[1] flow:functioncall has:1 depth:1 f:a.cpp l:75 cond:p
[2] flow:if has:1 depth:1 f:a.cpp l:80 cond: ( (number > 0))
[3] flow:elseif has:1 depth:1 f:a.cpp l:84 cond: (number < 0)
[4] flow:else has:0 depth:1 f:a.cpp l:88 cond:
[5] flow:if has:1 depth:1 f:a.cpp l:97 cond:( example(c,d) )
[6] flow:for has:1 depth:1 f:a.cpp l:104 cond:( a = 10;            a < 20; a = a + 1 )
[7] flow:while has:1 depth:1 f:a.cpp l:110 cond:( a < 25 )
[8] flow:do has:0 depth:1 f:a.cpp l:115 cond:
[9] flow:if has:1 depth:2 f:a.cpp l:117 cond:(a>3)
[10] flow:else has:0 depth:2 f:a.cpp l:119 cond:
[11] flow:while has:1 depth:1 f:a.cpp l:123 cond:( a < 2 )
[12] flow:switch has:1 depth:1 f:a.cpp l:125 cond:( a )
[13] flow:case has:1 depth:2 f:a.cpp l:126 cond: 1
[14] flow:break has:0 depth:2 f:a.cpp l:128 cond:
[15] flow:case has:1 depth:2 f:a.cpp l:129 cond: 8
[16] flow:break has:0 depth:2 f:a.cpp l:131 cond:
[17] flow:case has:1 depth:2 f:a.cpp l:132 cond: 3
[18] flow:case has:1 depth:2 f:a.cpp l:133 cond: 4
[19] flow:break has:0 depth:2 f:a.cpp l:135 cond:
[20] flow:default has:0 depth:2 f:a.cpp l:136 cond:
[21] flow:break has:0 depth:2 f:a.cpp l:137 cond:
[22] flow:functioncall has:1 depth:1 f:a.cpp l:145 cond:getline
[23] flow:if has:1 depth:1 f:a.cpp l:146 cond:(animal == "Squirrel")
[24] flow:if has:1 depth:2 f:a.cpp l:148 cond: ( (animal == "Ash Borer")               || (animal == "dog") )
[25] flow:functioncall has:1 depth:3 f:a.cpp l:151 cond:last
[26] flow:else has:0 depth:2 f:a.cpp l:152 cond:
[27] flow:do has:0 depth:1 f:a.cpp l:159 cond:
[28] flow:functioncall has:1 depth:2 f:a.cpp l:162 cond:last
[29] flow:while has:1 depth:1 f:a.cpp l:163 cond:(0)
[30] flow:switch has:1 depth:1 f:a.cpp l:165 cond:(a + 4)
[31] flow:case has:1 depth:2 f:a.cpp l:166 cond: ClassA::None
[32] flow:break has:0 depth:2 f:a.cpp l:168 cond:
[33] flow:case has:1 depth:2 f:a.cpp l:169 cond: ClassA::If
[34] flow:if has:1 depth:3 f:a.cpp l:172 cond: ( a < 7 )
[35] flow:break has:0 depth:2 f:a.cpp l:176 cond:
[36] flow:case has:1 depth:2 f:a.cpp l:177 cond: ClassA::Else
```


```cpp  a.cpp
#include <iostream>
#include <string>
using namespace std;

void last();

/* start of Enclosing class declaration */
class Enclosing {
   private:
       int x;

   /* start of Nested class declaration */
   class Nested {
      int y;
      void NestedFun(Enclosing *e) {
        cout<<e->x;  // works fine: nested class can access
                     // private members of Enclosing class
      }
   }; // declaration Nested class ends here
}; // declaration Enclosing class ends here


template <class T>
T GetMax (T a, T b) {
  T result;
  result = (a>b)? a : b;
  return (result);
}


class DerivedA {
public:
    int a ;
    int b ;
    DerivedA(int a=0,int b=0){}
};

class ClassA : public DerivedA {
private:
    int c ;
    int d ;
public:
    enum FF {
        None,
        If,
        Else,
    };
    ClassA(int aa=0,int bb=0):c(aa),d(bb) {} 
    void classAfunc(int t);
};

void ClassA::classAfunc(int t){
    if(t == 3){
        t=1;
    } else 
        t =4;
    return;
}

int example(int& aa,int &bb)
{
	if ( aa < bb ) {
		aa = bb;
	}
	else {
		aa = -bb;
        ClassA::classAfunc(aa);
	}

	return aa;
}

int main()
{
    ClassA p(0,0);

   	int number;
    cout << "Enter an integer: ";
    cin >> number;
    if ( (number > 0))
    {
        cout << "You entered a positive integer: " << number << endl;
    }
    else if (number < 0)
    {
        cout<<"You entered a negative integer: " << number << endl;
    }
    else
    {
        cout << "You entered 0." << endl;
    }
    cout << "This line is always printed.";
	
    /* { }
     */
	int c=10,d=30;
	if( example(c,d) ) c++;
    /** { ( ) }
     */


	// for loop execution
    int a ;
   for( a = 10; 
           a < 20; a = a + 1 ) {
      cout << "for loop value of a: " << a << endl;
   }

   // while loop execution
   while( a < 25 ) {
      cout << "while loop value of a: " << a << endl;
      a++;
   }

   do {
      cout << "while loop value of a: " << a << endl;
      if(a>3){
          a++;
      } else {
          a+=3;
      }
      a++;
   } while( a < 2 ) ;

   switch( a ){
       case 1:
           cout << "1" << endl;
           break;
       case 8  :
           cout << "8" << endl;
           break;
        case 3 : 
        case 4 : 
           cout << "1" << endl;
           break;
       default:
           break;
   };


	// nested if
   string animal;
   string branch;
   cout << "Enter Animal : ";
   getline(cin, animal);
   if(animal == "Squirrel") {
	   branch = "Left";
	   if ( (animal == "Ash Borer")
			   || (animal == "dog") ){
		   branch = "Right";
           last();
	   } else {
		   branch = "Undefined";
	   }
	   cout << branch;
   }

   a = 0;
   do {
       a++;
	   branch = "fined";
       last();
    } while(0);

    switch(a + 4){
        case ClassA::None:
            cout << "None"; 
            break;
        case ClassA::If:
            cout << "If"; 
            {
                if ( a < 7 ) {
                    a = 7;
                }
            }
            break;
        case ClassA::Else:
            cout << "Else"; 
            break;
        default:
            cout << "Default"; 
            break;
    };

   {
       int i=5, j=6, k;
       long l=10, m=5, n;
       k=GetMax<int>(i,j);
       n=GetMax<long>(l,m);
       cout << k << endl;
       cout << n << endl;
   }
  return 0;
}

void last()
{
    int aa =1 , bb = 3;
	if ( aa < bb ) {
		aa = bb;
	}
}
```
