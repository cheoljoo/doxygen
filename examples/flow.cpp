#include <iostream>
#include <string>
using namespace std;

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
	   } else {
		   branch = "Undefined";
	   }
	   cout << branch;
   }

   a = 0;
   do {
       a++;
	   branch = "fined";
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
