/***********************************************************************************************************
Polynomial Root Finder
Roy Chancellor

Description:  Finds all roots (when possible) of a polynomial function from degree 1 (linear) to degree 8.
The user enters the degree  for a polynomial function and all its coefficients and the program finds the roots for f(x) = 0.

Methodology:
Implements simple arithmetic for degree 1, the quadratic formula for degree 2, and Newton's method for real roots when
the degree is 3 or higher.  To identify real roots for cubic or higher, the program scans a domain for sign changes and uses
linear interpolation to create guesses, then feeds the guesses into Newton's method.  If all roots are not found, the program then uses
synthetic division to divide all real roots into the original polynomial to reduce it.  If the reduced degree is 2, the program uses the
quadratic formula to find the remaining two roots.  If the reduced degree is 4 or higher, the program uses Bairstow's method to find the
remaining roots.
************************************************************************************************************/

//C++ HEADERS
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <cctype>  //for isdigit
#include <cstdlib>  //for exit in cAddSub
using namespace std;

//Constant definitions
#define MAXDEGREE 8
#define MINDEGREE 1
#define REAL 1
#define CPLX -1
#define DBLROOT 0
#define MAXITER 1000
#define MENUMIN 1
#define MENUMAX 2
#define FINDPOLYROOTS 1
#define EXIT 2
#define LINEAR 1
#define QUADRATIC 2
#define LINCROSSES 1
#define QUADCROSSES 2
#define POLYNOM 3
#define ZEROTOL 0.00000001
#define INTTOL 0.000001
#define NUMSCANPOINTS 200.0
#define SIGDIGMAX 6
#define ROOTPREC 6
#define FX 1
#define FNEGX -1
#define RE 0
#define IM 1
#define PROGREV 17

//STRUCTURE DEFINITIONS
typedef struct cplx {  //defines a structure to hold and operate on complex numbers
	double re;
	double im;
} cplx;

//PRIMARY FUNCTION PROTOTYPES
bool displayMenu( void );  //Displays the menu of options
int polyConfig( double coeff[], int *numZeroRoots );  //gets the polynomial degree from the user, then find the number of zero roots, if any
bool getPolyCoeff( int degree, double coeff[] );  //called from polyConfig, gets the coefficients for the polynomial from the user
int quadFormula( double coeff[], double roots[] );  //computes b/2a and sqrt(b^2 - 4ac)/2a from the quadratic formula.  Returns the root type (REAL, CPLX, DBLROOT) if successful and 999 if not
int polyFindCrossings( int degree, double coeff[], double guess[] );  //Finds all axis crossings between the +/- limits of the largest possible rational root...NEEDS IMPROVEMENT FOR DOMAIN OF SEARCH
bool polyFindRealRoots( int degree, double coeff[], double guess[], int numCrossings, double roots[], double error[], int rootFound[] );  //Newton's method for finding real roots of polynomial functions of degree 3 and higher
bool polyFindRemaining( int numRootsRemain, int numZeroRoots, int degree, double coeff[], double roots[], struct cplx cplxRoots[MAXDEGREE], int rootFound[] );  //Finds all remaining roots, either by reducing directly to a quadratic or using Bairstow's method.
int quadPrintRoots( int rootIndex, double coeff[] );  //prints the two roots of a quadratic function using proper precision and complex notation when appropriate
bool printRootErrorOutOfTol( double rootError );  //prints a root error out of tolerance message
bool printComplexRoots( int index, struct cplx z );  //prints a complex number formatted properly based on the real and imaginary value
int polyPrintRealRoots( int degree, double coeff[], double roots[], int rootFound[], int numCrossings, int numZeroRoots );  //prints the real polynomial roots
struct cplx polyFuncEval( int degree, double coeff[], struct cplx z );  //calculates f(z) using a running sum, where 'z' is a complex number
double polyDerivEval( int degree, double coeff[], double x );  //calculates f'(x), where x is the real part of a complex number z, using a running sum
bool polyPrintFunc( int degree, double coeff[], int reflect, int numZeroRoots );  //prints the polynomial term-by-term in descending order of degree
bool polySynDiv( int degree, double coeff[], double factor, double reducedCoeff[] );  //performs synthetic division of a polynomial by a root and stores the reduced polynomial in reducedCoeff
bool descartesRule( int degree, double coeff[] );  //implements Descartes' Rule of Signs to determine the possible number of positive, negative, and (hence) complex conjugate pairs of roots for the user's polynomial
bool bairstowsMethod( int degree, double coeff[], struct cplx z[MAXDEGREE], int rootFound[] );  //uses Bairstow's method to find complex roots of a polynomial when it has no real roots
bool bairstowSetRoots( struct cplx z[MAXDEGREE], double r, double s, int rootIndex, int rootFound[] );  //sets the quadratic roots each time a pair is found using Bairstow's method

//UTILITY FUNCTION PROTOTYPES
int getNumber( char type, string prompt, double *dvar );  //gets a double floating point value or an integer from the user; type = 'r' allows any real number and type = 'i' forces an integer
string IntToString ( int Number );  //converts a number to a string
int sigDigits( double dval );  //determines and returns the number of significant digits of dval
bool clearScreen( void );  //clears the screen by writing a preset number of line feeds
int sumArray( int array[] );  //sums up all the elements in an INTEGER array; returns the sum as an integer
bool printRootHeader( void );  //a simple header for printing a table of roots
struct cplx cMult( struct cplx, struct cplx );  //multiplies two complex numbers, z1 and z2, and returns their product as a complex number
struct cplx cAddSub( struct cplx, struct cplx, int );  //adds or subtracts two complex numbers based in the value of 'operation' and returns the sum or difference as a complex number
struct cplx cPow( struct cplx z, int exp );  //raises a complex number to a power by multiplying it by itself 'power' number of times (power must be an integer) and returns the power as a complex number

int main() {  //Program currently has no command line arguments; returns true when it completes properly
	int keepGoing, menuCode;
	int degree, allRootsZero, numZeroRoots, numRootsRemain, numCrossings;
	int rootFound[MAXDEGREE];
	double tmpDouble;  //used as temporary storage for the getNumber routine
	double coeff[MAXDEGREE + 1], guess[MAXDEGREE], roots[MAXDEGREE], error[MAXDEGREE];
	struct cplx cplxRoots[MAXDEGREE];

	clearScreen();

	cout << "****** Polynomial roots Finder, Rev " << PROGREV << " (C) 2013-2016, Mr. Chancellor ******";
	cout << "\n\nProgram will find all roots of a polynomial of degree " << MINDEGREE << " up to " << MAXDEGREE;

	do {  //main program loop
		//display the main menu and ask the user for a selection; repeat until the user enters a valid selection
		keepGoing = displayMenu();
		do
			menuCode = getNumber( 'i', " --> ", &tmpDouble );
		while( menuCode < MENUMIN || menuCode > MENUMAX );  //ensures the user enters a valid menu item

		//operate on the user's menu choice
		switch( menuCode ) {
			case FINDPOLYROOTS:  //find the root(s) of the polynomial function
				//get the polynomial degree and its coefficients
				degree = polyConfig( coeff, &numZeroRoots );
				//FROM HERE ON, 'degree' MEANS THE ORIGINAL DEGREE OR THE DEGREE REDUCED BY THE NUMBER OF ZERO ROOTS (i.e. number of trailing zero coefficients)
				switch( degree ) {
					case LINEAR:
						//calculate the single root here
						roots[0] = -coeff[1] / coeff[0];  //the solution to f(x) = ax + b = 0 --> x = -b/a; a <> 0
						//print the function and its root
						keepGoing = polyPrintRealRoots(degree, coeff, roots, rootFound, LINCROSSES, numZeroRoots );
						break;
					case QUADRATIC:
						//print the roots (the function calculates the roots locally)
						keepGoing = polyPrintRealRoots(degree, coeff, roots, rootFound, QUADCROSSES, numZeroRoots );
						break;
					default:  //degree == POLYNOM
						//Determine the possible number of +, -, cplx roots for the original/reduced polynomial
						allRootsZero = descartesRule( degree, coeff );  //returns true if all roots are zero (and states so) and false if not all roots are zero
						if( !allRootsZero ) {  //then f(x) != ax^n and NOT all n roots are zero
							//Determine the number of x-axis crossings of the polynomial (REAL ROOTS), if any
							numCrossings = polyFindCrossings( degree, coeff, guess );
							//For each x-axis crossing approximation, use Newton's method to find the closest root
							keepGoing = polyFindRealRoots( degree, coeff, guess, numCrossings, roots, error, rootFound );
							//Print REAL roots and/or fail messages to the user
							keepGoing = polyPrintRealRoots( degree, coeff, roots, rootFound, numCrossings, numZeroRoots );
							//Find and print all remaining roots (COMPLEX CONJUGATE ROOTS), if any
							numRootsRemain = degree - sumArray(rootFound);
							if( numRootsRemain > 0 ) {  //then not all roots found
								if( numRootsRemain % 2 == 0 )  //then numRootsRemain is EVEN, so go find the remaining roots
									keepGoing = polyFindRemaining( numRootsRemain, numZeroRoots, degree, coeff, roots, cplxRoots, rootFound );
								else //then the number of remaining roots is ODD, which means at least one real root was missed (complex roots of polynomials with real coefficients come in conjugate PAIRS)
									cout << "\n\n\tSOMETHING BAD HAS HAPPENED:  NUMBER OF REMAINING ROOTS (" << numRootsRemain << ") IS ODD, WHICH CAN'T BE AT THIS STAGE!!!";
							} //end if
							else
								cout << "\n\t****************************************************************************";
						}  //end if
						break;
				}  //end degree switch
				break;
			case EXIT:  //exit
				keepGoing = false;
				break;
		}
		cout << "\n\n\n";  //put some blank space before the menu appears again
	} while( keepGoing );  //end of main program loop

	return true;
} //end main

//gets the polynomial degree and its coefficients from the user, then find the number of zero roots, if any
int polyConfig( double coeff[], int *numZeroRoots )
{
	int i, degree;
	double tmpDouble;

	//get the degree from the user and validate it is in bounds
	do
		degree = getNumber( 'i', "\n\nEnter the polynomial degree as an integer from " + IntToString(MINDEGREE) + " to " + IntToString(MAXDEGREE) + ":  ", &tmpDouble );
	while( degree > MAXDEGREE || degree < MINDEGREE );  //ensure the user enters a valid degree

	//get the polynomial coefficients based on the degree
	getPolyCoeff( degree, coeff );  //returns a boolean, but not using it for anything

	//if the degree is 3 or higher, determine if the user has entered zero coefficients at the end of the polynomial and if so, how many; reduce the effective degree of the polynomial by this amount (this is the number of zero roots that will be factored out)
	*numZeroRoots = 0;
	if( degree >= POLYNOM ) {
		i = degree;  //coeff[degree] contains the constant term
		//count all the trailing zeros, but don't count the leading coefficient (coeff[0]), as it will always be nonzero
		while( coeff[i] == 0 && i > 0 ) {  //starts with constant and goes backward
			++*numZeroRoots;
			--i;
		}
		//reduce the degree by the number of trailing zeros; this effectively factors out x^a, where a = number of trailing zeros
		if( *numZeroRoots > 0  /*user entered at least one trailing zero*/ && *numZeroRoots < degree /*user entered all zero coefficients*/ )
			degree -= *numZeroRoots;
	}

	return degree;
}  //end polyConfig

//Configure the polynomial root problem by getting the degree and coefficients from the user
bool getPolyCoeff( int degree, double coeff[] )
{
	int i;
	double tmpDouble;

	//clearScreen();

	//LEADING COEFFICIENT (must be nonzero for a function of any degree)
	cout << "\n\nEnter the NONZERO leading coefficient for the polynomial function";
	do {
		switch( degree ) {  //display a custom prompt based on the degree
			case LINEAR:
				cout << "\n\tslope";
				break;
			case QUADRATIC:
				cout << "\n\ta";
				break;
		}
		getNumber( 'r', " --> ", &tmpDouble );  //returns an integer but not using it for anything
		if( tmpDouble == 0 )
			cout << "\n\tMUST ENTER A NONZERO LEADING COEFFICIENT\n";
		else
			coeff[0] = tmpDouble;
	} while( tmpDouble == 0 );  //prevents a zero leading coefficient

	//REMAINING COEFFICIENTS based on the problem type
	if( degree == LINEAR ) {  //custom prompt for linear y-intercept
		cout << "\n\ty-intercept, b";
		getNumber( 'r', " --> ", &tmpDouble );
		coeff[1] = tmpDouble;  //stores the y-intercept
	}
	else if( degree == QUADRATIC ) {  //custom prompts for quadratic b, c
		cout << "\n\tlinear term, b";
		getNumber( 'r', " --> ", &tmpDouble );
		coeff[1] = tmpDouble;  //stores the linear term
		cout << "\n\tconstant, c";
		getNumber( 'r', " --> ", &tmpDouble );
		coeff[2] = tmpDouble;  //stores the constant
	}
	else {  //POLYNOM
		//get the remaining coefficients based on the degree
		cout << "\n\nEnter the remaining coefficients, starting with the x^" << degree - 1 << " term and proceeding down in power...";
		for( i = 1; i <= degree; ++i ) {  //needs to include the degree index because there is always one more coefficient than the degree
			cout << "\n\ta" << degree - i;
			getNumber( 'r', " --> ", &tmpDouble );
			coeff[i] = tmpDouble;  //stores the coefficients from leading to constant
		}
	}

	return true;
} //end getPolyCoeff

//Finds real roots of a polynomial of degree 3 or higher using Newton's method
bool polyFindRealRoots( int degree, double coeff[], double guess[], int numCrossings, double roots[], double error[], int rootFound[] )
{
	int i, curRoot;
	double fx, dfdx;
	struct cplx z;
	bool atLeastOneRootFound = false;

	//Initialize the rootFound array with false to be able to determine the number of roots found later
	for( i = 0; i < MAXDEGREE; ++i )
		rootFound[i] = false;

	cout << "\n\n\tFINDING ANY REAL ROOTS OF f(x)...";
	if( numCrossings == 0 )
		cout << "\n\tNone found!";
	else { //at least one real root exists, so find each with Newton's method
		for(curRoot = 0; curRoot < numCrossings; ++curRoot )  //loops = number of x-axis crossings identified previously
		{
			//print the Newton's method iterations for the user
			cout.precision( sigDigits(guess[curRoot]) );
			cout << "\n\n\tRoot " << curRoot + 1 << ":  Guess = " << guess[curRoot] << " and error tolerance = ";
			cout.precision( sigDigits(ZEROTOL) );
			cout << ZEROTOL;
			cout << "\n\n\tIterating...\n";
			cout << "\n\t\tx\t\tf(x)";
			cout << "\n\t\t------------------------";

			//Evaluate the polynomial function at the guess value of x and calculate the current error
			z.re = guess[curRoot];
			z.im = 0.0;
			fx = polyFuncEval( degree, coeff, z ).re;  //function requires a complex number input and returns a complex number, but only need the REAL (.re) part here
			error[curRoot] = fabs( fx );
			cout.setf( ios::fixed, ios::floatfield );  //set the float field to fixed so the tables are uniform
			cout.precision(ROOTPREC);
			cout << "\n\t\t" << z.re;
			cout << "\t" << ((z.re < 0) ? "" : "") << fx;

			//************* START NEWTON'S METHOD *************
			i = 0;
			while( i < MAXITER && error[curRoot] >= ZEROTOL ) {  //continue with Newton's method until the root is found or the maximum number of iterations have occurred
				dfdx = polyDerivEval( degree, coeff, z.re );  //compute the derivative of the polynomial at the current value of x
				if( dfdx != 0.0 )
					z.re -= fx / dfdx;  //Newton's formula (ref Thomas' Calculus section 4.7)
				else
					cout << "\n\nERROR:  f'(x) = 0!!!!!";

				//compute the value of the polyFuncEval at the (hopefully) NEW AND IMPROVED value of x just calculated using Newton's method
				fx = polyFuncEval( degree, coeff, z ).re;  //function requires a complex number input and returns a complex number, but only need the REAL (.re) part here
				cout << "\n\t\t" << z.re << "\t" << ((z.re < 0) ? "" : "") << fx;  //print the new value of x, and f(x) into the table
				error[curRoot] = fabs( fx );  //compute the new value of |error|
				++i;
			}
			//************* END NEWTON'S METHOD***************

			//At this point either a root was found or the iterations have been exceeded and no root is found
			if( i < MAXITER && error[curRoot] < ZEROTOL ) {
				roots[curRoot] = z.re;
				rootFound[curRoot] = true;
				atLeastOneRootFound = true;
			}
			else if( i >= MAXITER )
				rootFound[curRoot] = false;
		} //end for
	}  //end else
    cout.unsetf( ios::floatfield );  //unset the float field from fixed to default
	return atLeastOneRootFound;
} //end polyFindRealRoots

//evaluates the polynomial function at a value of z using a running sum, where 'z' is a COMPLEX number
struct cplx polyFuncEval( int degree, double coeff[], struct cplx z )
{
	int i;
	struct cplx fx, prod, a;

	//initialize local complex variables, then compute the running sum for all terms >= power 2
	fx.re = 0.0;
	fx.im = 0.0;
	a.re = 0.0;
	a.im = 0.0;  //coefficients will always be REAL
	for( i = 0; i <= degree - 2; ++i) {  //add everything but the linear term and the constant
		a.re = coeff[i];
		prod = cMult( a, cPow(z, degree - i) );
		fx = cAddSub( fx, prod, 'a' );
	}

	//add the linear term (the cPow function fails when exponent is <= 2)
	a.re = coeff[degree - 1];
	prod = cMult( a, z );
	fx = cAddSub( fx, prod, 'a' );  //'a' means add

	//add the constant last (the cPow function fails when exponent is <= 2)
	a.re = coeff[degree];
	fx = cAddSub( fx, a, 'a' );

	return fx;
} //end polyFuncEval

//evaluates the derivative of the polynomial function at a REAL value of x using a running sum, where 'x' is a real number
double polyDerivEval( int degree, double coeff[], double x )
{
	int i;
	double dfdx;

	dfdx = 0.0;
	for( i = 0; i <= degree - 2; ++i)  //add everything but the constant
		dfdx += coeff[i] * (degree - i) * pow(x, degree - i - 1);  //the power rule for derivatives of polynomials

	//add the constant last (the pow function fails when exponent is zero)
	dfdx += coeff[degree - 1];

	return dfdx;
} //end polyDerivEval

//identifies all x-axis crossings in the domain of search which is based on the rational root theorem
//DOES NOT IDENTIFY MULTIPLE ROOTS, BUT BAIRSTOW'S METHOD WILL FIND THEM LATER
int polyFindCrossings( int degree, double coeff[], double guess[] )
{
	double yLow, yHigh, deltaX, minX, maxX;
	int i, constFound;
	struct cplx z;

	//initialize the guess array
	for( i = 0; i < degree; ++i )
		guess[i] = 0.0;

	//(For now) Use the rational root theorem to generate a domain of search for x-axis crossings and double the range to search for axis crossings

	//Find the first non-zero coefficient, starting with the constant term stored in coeff[degree].
	//Since all zero roots are factored out in "main", should never find a zero coefficient on the end of the reduced polynomial.
	constFound = false;
	i = degree;
	while( !constFound && i >= 1 ) {
		if( coeff[i] != 0 )
			constFound = true;
		else
			--i;
	}
	if( constFound )
		minX = -4.0 * fabs( coeff[i] / coeff[0] );  //coeff[i] contains the constant (or first non-zero coeff nearest constant term) and coeff[0] the leading coefficient
	else
		minX = -10.0;

	maxX = -minX;  //minX will always be negative, so make maxX its opposite
	deltaX = (maxX - minX) / NUMSCANPOINTS;  //divides the domain of search into a "large" number of points to look for x-axis crossings

	z.re = minX;  //start the search for axis crossing at the lowest possible rational root
	z.im = 0.0;
	i = 0;  //index of root crossings
	while( z.re <= maxX ) {  //DOES NOT IDENTIFY "TOUCHES" (MULTIPLE ROOTS)
		yLow = polyFuncEval( degree, coeff, z ).re;  //the function returns a complex number, but only need its real part
		z.re += deltaX;
		yHigh = polyFuncEval( degree, coeff, z ).re;
		if( (yLow <= 0.0 && yHigh >= 0.0) || (yLow >= 0.0 && yHigh <= 0.0) ) {  //then a x-axis cross occurred
			guess[i] = z.re + deltaX / 2.0;  //assume the crossing is half way between the x and x + delta X and make the guess this value.  Could use linear interpolation to make a better guess.
			++i;
		}
	}

	return i;  //the number of x-axis crossings in the domain of search
}  //end polyFindCrossings

//prints out all REAL roots in a formatted table
int polyPrintRealRoots( int degree, double coeff[], double roots[], int rootFound[], int numCrossings, int numZeroRoots )
{
	int i, origDegree;
	string chTime = "time", chPlural = "s";
	double rootError = 999.0;
	struct cplx z;

	//restore the original polynomial degree if there are multiplicities of zero roots
	origDegree = degree + numZeroRoots;

	//print header
	cout << "\n\n\n\t***************************************************************************";

	if( origDegree == LINEAR ) {
		//print the function and the root
		cout << "\n\tThe one root of f(x) = ";
		polyPrintFunc( LINEAR, coeff, FX, 0 );
		cout << " is:";
		printRootHeader();  //returns a boolean but not using it for anything
		//validate the root by evaluating f(root) and comparing it to zero
		z.re = roots[0];
		z.im = 0.0;
		rootError = polyFuncEval( LINEAR, coeff, z ).re - 0.0;
		if( fabs(rootError) > ZEROTOL )
			printRootErrorOutOfTol(rootError);
		cout.precision( sigDigits(roots[0]) );
		cout << "\n\t1\t" << roots[0];
		cout.precision( sigDigits(rootError) );
		cout << "\t\t\t" << rootError;
	}
	else if( origDegree == QUADRATIC ) {
		cout << "\n\tThe two roots of the quadratic function f(x) = ";
		polyPrintFunc( degree, coeff, FX, 0 );
		cout << " are:";
		printRootHeader();
		quadPrintRoots( origDegree, coeff );  //calculates and prints roots of a quadratic polynomial
	}
	else {  //origDegree == POLYNOM
		//STEP ONE:  print out the number of x-axis crossings
		if( numCrossings + numZeroRoots != 1 )
			chTime.append(chPlural); //becomes "times" for 0 and >= 2
		cout << "\n\tIdentified " << numCrossings + numZeroRoots << " " << chTime << " where f(x) = ";
		polyPrintFunc( degree, coeff, FX, numZeroRoots );
		cout << " crosses or touches the x-axis";

		if( numCrossings + numZeroRoots == 0 ) {
			if( numZeroRoots > 0 )
				cout << "\n\tAll " << degree << " nonzero roots are COMPLEX or MULTIPLE ROOTS";
			else
				cout << "\n\tAll " << origDegree << " roots are COMPLEX or MULTIPLE ROOTS";
		}
		else
			cout << "\n\tTherefore, " << origDegree - numCrossings - numZeroRoots << " COMPLEX roots remain";
		printRootHeader();

		//STEP 2:  print multiplicities of zero roots, if any
		if( numZeroRoots > 0 ) {  //then print out the number 0 that many times
			for( i = 0; i < numZeroRoots; ++i )
				cout << "\n\t" << i + 1 << "\t0";
		}

		//STEP 3:  print all nonzero REAL roots for the original or reduced (has zero roots) polynomial
		switch( degree ) {  //degree will be the original or the original reduced by the number of zero roots, if any
			case LINEAR:  //print the single remaining root after all the zero multiple roots
				cout.precision( sigDigits(roots[0]) );
				cout << "\n\t" << origDegree << "\t" << roots[0];
				cout << "\n\t***************************************************************************\n\n\n";
				break;
			case QUADRATIC:  //print the two remaining roots after all the zero multiple roots
				quadPrintRoots( origDegree, coeff );
				cout << "\n\t***************************************************************************\n\n\n";
				break;
			default:  //print any real roots found after all the zero multiple roots
				if( numCrossings > 0 ) {  //some or all nonzero roots are REAL
					for( i = 0; i < numCrossings; ++i ) {
						if( rootFound[i] == true ) {
							if( fabs( roots[i] - round(roots[i]) ) <= INTTOL )
								cout << "\n\t" << i + 1 + numZeroRoots << "\t" << round(roots[i]);
							else {
								cout.precision( sigDigits(roots[i]) );
								cout << "\n\t" << i + 1 + numZeroRoots << "\t" << roots[i] ;
							}
							//validate the root by evaluating f(root) and comparing it to zero
							z.re = roots[i];
							z.im = 0.0;
							rootError = polyFuncEval( degree, coeff, z ).re - 0.0;
							if( fabs(rootError) > ZEROTOL )
								cout << "\t\tROOT ERROR " << rootError << " > TOLERANCE";
						}
						else
							cout << "\n\t" << i + 1 << "\tFAILURE:  rootFound[" << i << "] = false; Maximum number of iterations exceeded before finding a real root";
					}  //end for
				}
				break;
		}  //end switch
	}  //end else

	//print footer
	if( origDegree <= QUADRATIC )
		cout << "\n\t****************************************************************************\n\n\n";

	return true;
} //end polyPrintRealRoots

//a simple header for printing a table of roots
bool printRootHeader( void )
{
	//print all known roots in a table
	cout << "\n\n\tIndex\tROOT";
	cout << "\n\t------------------------------";

	return true;
}  //end printRootHeader

//Prints the roots of a QUADRATIC function with the number of significant digits of the roots
int quadPrintRoots( int rootIndex, double coeff[] )
{
	int rootType;
	double realRoot1, realRoot2, bOver2a, radOver2a, rootError;
	double roots[2];
	struct cplx z;

	//Determine the root type and store b/2a and rad/2a in roots[0] and roots[1] respectively
	rootType = quadFormula( coeff, roots );
	bOver2a = roots[0];
	radOver2a = roots[1];

	//print the roots with the correct number of significant digits
	switch( rootType ) {
		case REAL:
			realRoot1 = bOver2a + radOver2a;
			realRoot2 = bOver2a - radOver2a;
			//print first root
			cout.precision( sigDigits(realRoot1) );
			cout << "\n\t" << rootIndex - 1 << "\t" << realRoot1;
			//validate the first root by evaluating f(root) and comparing it to zero
			z.re = realRoot1;
			z.im = 0.0;
			rootError = polyFuncEval( QUADRATIC, coeff, z).re - 0.0;
			if( fabs(rootError) > ZEROTOL )
				printRootErrorOutOfTol( rootError );
			//print second root
			cout.precision( sigDigits(realRoot2) );
			cout << "\n\t" << rootIndex << "\t" << realRoot2;
			//validate the second root by evaluating f(root) and comparing it to zero
			z.re = realRoot2;
			z.im = 0.0;
			rootError = polyFuncEval( QUADRATIC, coeff, z).re - 0.0;
			if( fabs(rootError) > ZEROTOL )
				printRootErrorOutOfTol( rootError );
			break;
		case CPLX:
			z.re = bOver2a;
			z.im = radOver2a;
			//print the root index and first root
			printComplexRoots( rootIndex - 1, z );  //returns an integer but not using it for anything
			//validate the first root by evaluating f(root) and comparing it to zero
			rootError = polyFuncEval( QUADRATIC, coeff, z).re - 0.0;
			if( fabs(rootError) > ZEROTOL )
				printRootErrorOutOfTol( rootError );
			//print the root index and second root
			z.im = -radOver2a;
			printComplexRoots( rootIndex, z );
			//validate the second root by evaluating f(root) and comparing it to zero
			rootError = polyFuncEval( QUADRATIC, coeff, z).re - 0.0;
			if( fabs(rootError) > ZEROTOL )
				printRootErrorOutOfTol( rootError );
			break;
		case DBLROOT:
			cout.precision( sigDigits(bOver2a) );
			cout << "\n\tDBL\t" << bOver2a;
			z.re = bOver2a;
			z.im = 0.0;
			//validate the double root by evaluating f(root) and comparing it to zero
			rootError = polyFuncEval( QUADRATIC, coeff, z).re - 0.0;
			if( fabs(rootError) > ZEROTOL )
				printRootErrorOutOfTol( rootError );
			break;
	}

	return true;
} //end quadPrintRoots

bool printRootErrorOutOfTol( double rootError )
{
    cout << "\t\tROOT ERROR " << rootError << " > TOLERANCE";

    return true;
}

//prints a complex number formatted properly based on the real and imaginary value
//must check to determine if the value passed is real or complex (i.e. nonzero imaginary part)
bool printComplexRoots( int index, struct cplx z )
{
	bool isComplex;

	isComplex = true;  //assume the passed value has a nonzero imaginary part until found different
	if( fabs(z.im) <= ZEROTOL )
		isComplex = false;

	if( isComplex ) {
		if( fabs(z.re) <= ZEROTOL ) {  //then real part is zero
			if( fabs(z.im) - 1.0 <= INTTOL ) {  //then imag part is +/- i only
				if( z.im < 0 )
					cout << "\n\t" << index << "\t- i";
				else
					cout << "\n\t" << index << "\t+ i";
			}
			else {  //then imag part is any real number <> 1
                cout.precision( sigDigits(z.im) );
				if( z.im < 0 )
					cout << "\n\t" << index << "\t-" << fabs(z.im) << " i";
				else
					cout << "\n\t" << index << "\t+" << fabs(z.im) << " i";
			}
		}
		else {  //then real part is any non-zero number
            cout.precision( sigDigits(z.re) );
			if( fabs(z.im) - 1.0 <= INTTOL ) {  //then imag part is +/- i only
				if( z.im < 0 )
					cout << "\n\t" << index << "\t" << z.re << " - i";
				else
					cout << "\n\t" << index << "\t" << z.re << " + i";
			}
			else {  //then imag part is any real number <> 1
                cout.precision( sigDigits(z.im) );
				if( z.im < 0 )
					cout << "\n\t" << index << "\t" << z.re << " - " << fabs(z.im) << " i";
				else
					cout << "\n\t" << index << "\t" << z.re << " + " << fabs(z.im) << " i";
			}
		}
	}  //end if
	else {  //the passed value is actually REAL
		if( fabs( z.re - round(z.re) ) <= INTTOL )  //then real part is an integer
			cout << "\n\t" << index << "\t" << round(z.re);
		else {  //then imag part is any real number <> 1
			cout.precision( sigDigits(z.re) );
			cout << "\n\t" << index << "\t" << z.re;
		}
	}

	return true;
}  //end printComplexRoots

//Computes the roots of the quadratic equation based on the type of root (real or complex)
int quadFormula( double coeff[], double roots[] )
{
	int rootType;  //a value that will contain 1, -1, or 0 and becomes a multiplier in the radical part of the quad formula
	double discrim, a, b, c;
	double twoA, bOver2a, radOver2a;
	char ch;

	//set the coefficients a, b, c for ease of use and clarity
	a = coeff[0];
	b = coeff[1];
	c = coeff[2];
	twoA = 2.0 * a;
	//calculate the (always) real part of the roots
	bOver2a = -b / twoA;
	//calculate the discriminant to decide what type of roots will result
	discrim = b*b - 4*a*c;
    //Determine the type of roots based on the sign of the discriminant
	if( discrim > 0 )
		rootType = REAL;  //equals 1
	else if( discrim < 0 )
		rootType = CPLX;  //equals -1
	else if( discrim == 0 )
		rootType = DBLROOT;  //equals 0
	else {
		//in the very unlikely event that the discriminant is not a number
		cout << "\n\nSERIOUS ERROR:  Unable to determine the discriminant.  Press \"Enter\" to quit.";
		cin.get(ch);  //getch();
		rootType = 999;
	}

	//Calculate the radical part of the roots.  Multiplying by the value of rootType takes care of the sign under the radical
	radOver2a = sqrt( discrim * rootType ) / twoA;  //rootType guarantees the radicand is a non-negative number so the SQRT function works properly

	//set each part of the two roots in the roots array
	roots[0] = bOver2a;
	roots[1] = radOver2a;

	return rootType;
} //end quadFormula

//prints a polynomial in descending order of exponent power, ignoring zero coefficient terms.  Can print f(x) or f(-x) using the 'reflect' variable (1 = f(x), -1 = f(-x))
bool polyPrintFunc( int degree, double coeff[], int reflect, int numZeroRoots )
{
	int i, signMult;

	if( numZeroRoots > 0 ) {  //then need to factor out x^a and call this function recursively to print out the reduced polynomial
		if( numZeroRoots == 1 )  //don't print the exponent
			cout << "x(" << numZeroRoots;
		else
			cout << "x^" << numZeroRoots << "(";
		polyPrintFunc( degree, coeff, reflect, 0 );  //RECURSIVE call with '0' for numZeroRoots to print out the reduced polynomial after factoring out x^a
		cout << ")";
	}
	else {
		//LEADING TERM
		if( reflect == FNEGX && (degree % 2) == 1 /*ODD powers: change sign*/ )
			signMult = -1.0;
		else
			signMult = 1.0;  //EVEN powers:  don't change sign
		if( degree >= QUADRATIC ) {
			//1 or -1 coefficient
			if( fabs(coeff[0]) == 1 ) {
				if( coeff[0]*signMult > 0 )
					cout << "x^" << degree;
				else
					cout << "-x^" << degree;
			}
			//all other non-zero values
			else {
				cout.precision( sigDigits(coeff[0]) );
				cout << coeff[0]*signMult << "x^" << degree;
			}
		}

		//POLYNOMIAL MIDDLE TERMS of power 'degree' - 1 down to power 2 when 'degree' is POLYNOM or higher
		if( degree >= POLYNOM ) {
			//print the remaining terms, except the linear and constant
			for( i = 1; i <= degree - 2; ++i ) {
				if( reflect == FNEGX && ((degree - i) % 2) == 1 /*ODD powers: change sign*/ )
					signMult = -1.0;
				else
					signMult = 1.0;	 //EVEN powers:  don't change sign
				//1 or -1 coefficients
				if( coeff[i]*signMult == 1 )
					cout << " + x^" << degree - i;
				else if( coeff[i]*signMult == -1 )
					cout << " - x^" << degree - i;
				//all other nonzero values
				else {
                    cout.precision( sigDigits(coeff[i]) );
					if( coeff[i]*signMult > 0 )
						cout << " + " << fabs( coeff[i] ) << "x^" << degree - i;
					else if( coeff[i]*signMult < 0 )
						cout << " - " << fabs( coeff[i] ) << "x^" << degree - i;
				}
			}
		}

		//LINEAR TERM
		if( reflect == FNEGX )  //linear terms always get their sign changed for f(-x)
			signMult = -1.0;
		else
			signMult = 1.0;
		if( degree >= QUADRATIC ) {  //won't print a term if the coefficient is zero
			//1 or -1 coefficients
			if( fabs(coeff[degree - 1]) == 1 ) {
				if( coeff[degree - 1]*signMult < 0 )
					cout << " - x";
				else if( coeff[degree -1]*signMult > 0 )
					cout << " + x";
			}
			//all other non-zero values
			else {
				cout.precision( sigDigits(coeff[degree - 1]) );
				if( coeff[degree - 1]*signMult < 0 )
					cout << " - " << fabs( coeff[degree - 1] ) << "x";
				else if( coeff[degree -1]*signMult > 0 )
					cout << " + " << fabs( coeff[degree - 1] ) << "x";
			}
		}
		else {  //the function is LINEAR
			//1 or -1 coefficients
			if( fabs(coeff[degree - 1]) == 1 ) {
				if( coeff[degree - 1]*signMult < 0 )
					cout << "-x";
				else if( coeff[degree - 1]*signMult > 0 )
					cout << "x";
			}
			//all other non-zero values
			else {
				cout.precision( sigDigits(coeff[degree - 1]) );
				if( coeff[degree - 1]*signMult < 0 )
					cout << "-" << fabs( coeff[degree - 1] ) << "x";
				else if( coeff[degree - 1]*signMult > 0 )
					cout << fabs( coeff[degree - 1] ) << "x";
			}
		}

		//CONSTANT TERM (prints nothing if it is zero)
		cout.precision( sigDigits(coeff[degree]) );
		if( coeff[degree] < 0 )
			cout << " - " << fabs( coeff[degree] );
		else if( coeff[degree] > 0 )
			cout << " + " << fabs( coeff[degree] );
	} //end else

	return true;
} //end polyPrintFunc

//Reduces the original polynomial by factoring out all previously found real roots using synthetic division.
//If the depressed polynomial is quadratic, finds roots using the quadratic formula, otherwise uses Bairstow's method.
bool polyFindRemaining( int numRootsRemain, int numZeroRoots, int degree, double coeff[], double roots[], struct cplx cplxRoots[MAXDEGREE], int rootFound[] )
{
	int i, j, degreeSynDiv, numRootsFound;
	double coeffDiv[MAXDEGREE + 1], coeffQuo[MAXDEGREE], rootError;

	//copy the original coefficients into a dividend array of coefficients and a quotient array of coefficients before performing synthetic division
	for( i = 0; i <= degree; ++i ) {
		coeffDiv[i] = coeff[i];
		coeffQuo[i] = coeff[i];
	}

	//perform synthetic division of the original polynomial by each real root previously found and determine whether the reduced polynomial is quadratic or degree 4 or higher
	degreeSynDiv = degree;  //a running degree as the original polynomial is reduced by division
	numRootsFound = sumArray( rootFound );  //the number of REAL roots found previously
	for( i = 0; i < numRootsFound; ++i ) {
		polySynDiv( degreeSynDiv, coeffDiv, roots[i], coeffQuo );  //returns a boolean but not using it for anything
		//make the divisor coefficients for the next division the quotient coefficients of the current division
		//(not including the remainder)
		for( j = 0; j <= degreeSynDiv - i; ++j )
			coeffDiv[j] = coeffQuo[j];
		degreeSynDiv -= 1;  //decrement the degree for the next synthetic division
	}  //end for

	if( numRootsRemain == QUADRATIC ) {
		//print the remaining two roots
		quadPrintRoots( degree + numZeroRoots, coeffQuo );  //note that coeffQuo is passed, which contains the coefficients of the QUADRATIC depressed polynomial
		cout << "\n\t*****************************************************************************";
	}
	else {  //then numRootsRemain >= 4 (guaranteed because this function only gets called when numRootsRemain > 0 and
            //even, therefore since complex roots come in conjugate pairs, and numRemainRoots != 2, must be >= 4)
		//find remaining roots using Bairstow's method on the depressed polynomial after dividing all real roots found previously
		bairstowsMethod( degreeSynDiv , coeffQuo, cplxRoots, rootFound );
		if( numRootsRemain - sumArray(rootFound) == 0 ) {  //then Bairstow's method found all remaining roots
			for( i = 0; i < numRootsRemain; i += 2 ) {
				printComplexRoots( degree - numRootsRemain + i + 1, cplxRoots[i] );
				rootError = polyFuncEval( degree, coeff, cplxRoots[i] ).re - 0.0;
				if( fabs(rootError) > ZEROTOL )
					printRootErrorOutOfTol(rootError);
				printComplexRoots( degree - numRootsRemain + i + 2, cplxRoots[i+1] );
				rootError = polyFuncEval( degree, coeff, cplxRoots[i+1] ).re - 0.0;
				if( fabs(rootError) > ZEROTOL )
					printRootErrorOutOfTol(rootError);
			}
		}
		else if( numRootsRemain - sumArray(rootFound) > 0 )
			cout << "\n\tBairstow's method FAILED to find all " << numRootsRemain << " remaining roots!";
		else
			cout << "\n\tERROR!!!! numRootsRemain < 0!!!";  //should never get here
		cout << "\n\t*****************************************************************************";
	}

	return true;
} //end polyFundRemainingRoots

//function that performs synthetic division of a polynomial with coefficients stored in 'coeff' for a given factor and stores the reduced polynomial coefficients in 'reducedCoeff'
bool polySynDiv( int degree, double coeff[], double factor, double reducedCoeff[] )
{
	int i;

	//start the division as the leading coefficient ("bring it down")
	reducedCoeff[0] = coeff[0];

	//Perform the synthetic division algorithm
	for( i = 1; i <= degree - 1; ++i )
		reducedCoeff[i] = coeff[i] + (factor * reducedCoeff[i-1]);

	//remainder will be in reducedCoeff[degree];

	return true;
}  //end polySynDiv

//Implements Descartes' Rule of Signs to determine the possible number of positive, negative, and (hence) complex conjugate pairs of roots for the user's polynomial
//DROS:  number of positive roots is equal to the number of sign changes of the terms of f(x) or less by a multiple of 2
//number of negative roots is equal to the number of sign changes of the terms in f(-x) or less by a multiple of 2
//number of complex roots is equal to the difference between the polynomial degree and the sum of all combinations of possible positive and negative roots
bool descartesRule( int degree, double coeff[] )
{
	int i, j, p, n, signedCoeff, numChgFX, numChgFnegX, allRootsZero, keepGoing, chgP, chgN;
	double signsFX[degree + 1], signsFnegX[degree + 1];

	//initialize variables
	for( i = 0; i <= degree; ++i ) {
		signsFX[i] = 0.0;
		signsFnegX[i] = 0.0;
	}

	//count the number of non-zero coefficients in f(x) and f(-x) and place their signs (+1 or -1) into arrays
	j = 0;
	for( i = 0; i <= degree; ++i ) {
		if( coeff[i] != 0 ) {
			//coefficients of f(x)
			signsFX[j] = coeff[i] / fabs(coeff[i]);  //f(x)
			//coefficients of f(-x)
			if( (degree - i) % 2 == 0 )  //then term has an EVEN exponent (or is the constant term) and f(-x) = f(x), so maintain sign of coefficient
				signsFnegX[j] = coeff[i] / fabs(coeff[i]);
			else  //then term has an ODD exponent and f(-x) = -f(x), so change sign of coefficient
				signsFnegX[j] = -coeff[i] / fabs(coeff[i]);
			++j;
		}
	}
	signedCoeff = j;

	//determine the number of sign changes of the terms of f(x)
	numChgFX = 0;
	numChgFnegX = 0;
	if( signedCoeff >= 2 ) {  //then f(x) != ax^n only and DROS applies
		for( i = 0; i < signedCoeff-1; ++i ) {
			if( signsFX[i+1] != signsFX[i] )  //the values in the 'signs' arrays will be +1 or -1, allowing detection of a sign change
				++numChgFX;
			if( signsFnegX[i+1] != signsFnegX[i] )  //the values in the 'signs' arrays will be +1 or -1, allowing detection of a sign change
				++numChgFnegX;
		}
		//Based on the number of sign changes of f(x) and f(-x), determine all possibilities for roots of f(x) using DROS
		cout << "\n\n\tThe number of sign changes of f(x) = ";
		polyPrintFunc( degree, coeff, FX, 0 );
		cout << " is " << numChgFX;
		cout << "\n\n\tThe number of sign changes of f(-x) = ";
		polyPrintFunc( degree, coeff, FNEGX, 0 );
		cout << " is " << numChgFnegX;
		//allRootsZero = signedCoeff;

		//print the number of possible +, -, CPLX roots for the user
		p = numChgFX;
		n = numChgFnegX;
		cout << "\n\n\tBy Descartes' Rule of Signs, the possible root combinations of f(x) are:";
		cout << "\n\n\t\tPOS\tNEG\tCPLX OR MULTIPLE";
		cout << "\n\t\t--------------------------------";

		chgP = true;  //variables that set whether to change 'p' or 'n' in the combinations of roots table (true) or leave it constant (false)
		chgN = true;
		if( p < 2 )  //only report multiples of 2 as possibilities when the maximum number of roots is 2 or higher (i.e. don't decrement when the possible number of roots is 0 or 1)
			chgP = false;
		if( n < 2 )
			chgN = false;
		keepGoing = true;
		while( keepGoing ) {
			cout << "\n\t\t" << p << "\t" << n << "\t" << degree - (p + n);
			//stop for any combination of p = (0, 1) or n = (0, 1); otherwise decrement p and/or n by 2 and repeat
			if( (p == 0 && n == 0) || (p == 0 && n == 1) || (p == 1 && n == 0) || (p == 1 && n == 1) )
				keepGoing = false;
			if( chgP )
				p -= 2;
			if( chgN )
				n -= 2;
		}
		allRootsZero = false;  //essentially returning a Boolean that states that not all roots are zero
	}
	else if( signedCoeff == 1 ) {  //then f(x) = ax^n and all n roots are zero
		cout << "\n\n\t**********************************";
		cout << "\n\tAll " << degree << " roots of f(x) = ";
		polyPrintFunc( degree, coeff, FX, 0 );
		cout << " are ZERO";
		cout << "\n\t**********************************";
		allRootsZero = true;  //return a Boolean that states all roots are zero
	}
	else {  //should NEVER get here under normal circumstances
		cout << "\n\n\tERROR:  The number of signed coefficients is zero!!!  Polynomial is defined incorrectly!!!";
		allRootsZero = true;
	}

	return allRootsZero;
}  //end descartesRule

//displays the menu of options when called in 'main'
bool displayMenu( void )
{
	cout << "\n\n**********************************************" << endl;
	cout << "Main menu\n\n";
	cout << "(1)...Configure f(x) and find its roots\n";
	cout << "(2)...Exit";
	cout << "\n**********************************************" << endl;
	cout << "\nMake your selection ";

	return true;
} //end displayMenu

//Finds polynomial roots using Bairstow's method.  Invoked in this program when the number of remaining roots is even and > 2 (i.e. all remaining roots are complex).
//This algorithm was taken from Chapra and Canale, "Numerical Methods for Engineers", Second Ed., 1988, McGraw-Hill, pp. 658-665
bool bairstowsMethod( int degree, double coeff[], struct cplx z[MAXDEGREE], int rootFound[] )
{
	int i, rootIndex, iter, n, degreeGEPolynom;
	bool rootsFound;
	double r, s, epsA1, epsA2, determ, deltaR, deltaS;
	double a[MAXDEGREE + 2], b[MAXDEGREE + 2], c[MAXDEGREE + 2];

	//Store the original coefficients into the 'a' array.  The original coefficients are originally stored in the opposite order from the Bairstow procedure as written, so do the reversal first.
	for( i = degree; i >= 0; --i ) {
		a[i] = coeff[degree - i];
		rootFound[i] = false;  //initialize the rootFound boolean array
	}

	//Begin the root finding procedure, starting with the case where the degree is >= 3
	rootIndex = 0;
	iter = 0;
	n = degree;
	while( n >= POLYNOM && iter <= MAXITER ) {  //outer loop that continues until the reduced polynomial is quadratic or linear (it never should be linear in this program)
		degreeGEPolynom = true;
		iter = 0;
		epsA1 = 1.1 * ZEROTOL;
		epsA2 = 1.1 * ZEROTOL;
		rootsFound = false;

		//set initial guesses for r and s (these may not always be the best ones, but don't have a better way of generating them now)
		r = 1.0;
		s = 1.0;

		//perform Bairstow's method to find two roots
		while( !rootsFound && iter <= MAXITER ) {  //inner loop for root finding
			++iter;
			b[n] = a[n];
			b[n - 1] = a[n - 1] + r * b[n];
			c[n] = b[n];
			c[n - 1] = b[n - 1] + r * c[n];
			for( i = n - 2; i >= 0; --i ) {
				b[i] = a[i] + r * b[i + 1] + s * b[i + 2];
				c[i] = b[i] + r * c[i + 1] + s * c[i + 2];
			}

			determ = pow( c[2], 2 ) - ( c[3] * c[1] );

			if( determ != 0 ) {
				deltaR = ( -b[1] * c[2] + b[0] * c[3] ) / determ;
				deltaS = ( -b[0] * c[2] + b[1] * c[1] ) / determ;
				r += deltaR;
				s += deltaS;

				if( deltaR == 0 && deltaS == 0) { //If actual roots are guessed, then stop looking
					epsA1 = 0;
					epsA2 = 0;
				}
				else {
					epsA1 = fabs( ( deltaR / r ) * 100);
					epsA2 = fabs( ( deltaS / s ) * 100);
				}

				if( epsA1 <= ZEROTOL && epsA2 <= ZEROTOL) //check to see if roots were found
					rootsFound = true;
			}
			else if( determ == 0 ) {
				//cout << "\n Determinant = %lf", determ);
				++r;
				++s;
				iter = 0;
			}
			else if( determ == NAN ) {
				//cout << "\n\n A numeric error has occurred.");
				++r;
				++s;
			}
		}  //end while

		if( rootsFound ) {  //then store them in an array of complex numbers
			bairstowSetRoots( z, r, s, rootIndex, rootFound );  //returns a boolean but not using it for anything
			//increment the root index
			rootIndex += 2;
			//decrement the poly order now that two roots have been found and continue the process
			n -= 2;
			//change the original polynomial coefficients to the depressed coefficients
			for( i = 0; i <= n; ++i )
				a[i] = b[i + 2];
			//reset the iteration counter so the while loop will keep running
			iter = 0;
		}
	}  //end while

	if( iter <= MAXITER ) { //only proceed to the final quadratic part if the number of iterations is below the maximum
		if( !degreeGEPolynom ) //checks if the original/reduced polynomial was degree 2 or not
			rootIndex = 0;  //only if the entered/reduced polynomial was QUADRATIC, otherwise use the running root index
		if( n == QUADRATIC ) { //quadratic polynomial - either the final depressed one or the original
			r = -a[1] / a[2];
			s = -a[0] / a[2];
			bairstowSetRoots( z, r, s, rootIndex, rootFound );
		}
		else if( n == LINEAR) {
			z[rootIndex].re = -a[0] / a[1];
			z[rootIndex].im = 0.0;
			rootFound[rootIndex] = true;
		}
	}  //end if
	else {
		//cout << "\n\n\tRoots not found.  Try new guesses.\n";
		rootsFound = false;
	}

	return rootsFound;
}  //end bairstowsMethod

//sets the quadratic roots each time a pair is found using Bairstow's method
bool bairstowSetRoots( struct cplx z[MAXDEGREE], double r, double s, int rootIndex, int rootFound[] )
{
	int rootType;
	double quadCoeff[3], quadRoots[2];

	quadCoeff[0] = 1.0;
	quadCoeff[1] = -r;
	quadCoeff[2] = -s;
	rootType = quadFormula( quadCoeff, quadRoots );
	//store the roots in an array of complex variables
	if( rootType == CPLX ) {
		z[rootIndex].re = quadRoots[0];
		z[rootIndex].im = quadRoots[1];
		z[rootIndex + 1].re = quadRoots[0];
		z[rootIndex + 1].im = -quadRoots[1];
	}
	else {  //two real roots
		z[rootIndex].re = quadRoots[0] - quadRoots[1];
		z[rootIndex].im = 0.0;
		z[rootIndex + 1].re = quadRoots[0] + quadRoots[1];
		z[rootIndex + 1].im = 0.0;
	}
	//set the rootFound boolean flags
	rootFound[rootIndex] = true;
	rootFound[rootIndex + 1] = true;

	return true;
}  //end bairstowSetRoots

/************************************** UTILITY FUNCTIONS BELOW ***********************************************************************/
//Will get a numeric value from the user (type = 'r' will get a double and type = 'i' will force an integer)
//Stores a real number at the double variable address passed to it
//Returns an integer casting of the double value the user entered.  If type = 'i', this will be the integer you want.
int getNumber( char type, string prompt, double *dvar )
{
	int i, alphaPresent, bufLength, numDecimal, numDash;
	string buffer;

	//This do-while loop continues until the user correctly enters the type of data specified in the polyFuncEval call
	do
	{
		//initialize all flags, assuming the best until detected otherwise
		alphaPresent = 0;
		numDecimal = 0;
		numDash = 0;

		//prompt the user using the text passed to the polyFuncEval
		cout << prompt;
		getline(cin, buffer);
		if( buffer[0] == '\0' || buffer[0] == '\n' )  //Check to see if the user simply pressed Enter (the character '\0' represents Enter)
			alphaPresent = 1;
		else {
			//go through the user-entered string character-by-character to make sure there are no non-digit characters, except '.' or '-'
			bufLength = buffer.length();
			for( i = 0; i < bufLength; ++i ) {
				//check for a non-digit and allow negative sign to pass once.  Allow period to pass once only if type is real
				//the isdigit polyFuncEval returns 0 if the character is NOT a digit
				if( isdigit( buffer[i] ) == 0 ) {
					if( buffer[i] == '-' )
						++numDash;
					else if( buffer[i] == '.' )
						++numDecimal;
					else
						alphaPresent = 1;
				}
			}
			if( (numDash == 1 && buffer[0] != '-')
				|| numDash > 1
				|| (type == 'r' && numDecimal > 1)
				|| (type == 'i' && numDecimal > 0) )
				alphaPresent = 1;
		}
	} while( alphaPresent );

	//These statements convert the now-verified string into the double data type and integer data type
	stringstream(buffer) >> *dvar;
	//*dvar = atof( buffer );

	return (int)*dvar;
} //end getNumber

//converts an integer to a string
string IntToString ( int Number )
  {
     ostringstream ss;
     ss << Number;
     return ss.str();
  }

//multiplies a double value by 10 repeatedly and counts the number of times it takes to make an integer, up to a predefined maximum.  This value is the number of significant digits/
//returns the number of decimal digits PLUS the number of digits to the left of the decimal because of the way cout.precision works
int sigDigits( double dVal )
{
	int rightOfDec, leftOfDec;
	double tmpDbl;

	//FIND THE NUMBER OF DIGITS TO THE RIGHT OF THE DECIMAL
	//keep multiplying by 10 until an integer results.  The number of times multiplied by 10 is the number of significant digits, up to a #defined maximum value.
	rightOfDec = 0;
	tmpDbl = fabs(dVal);
	cout.precision(9);
	while( fabs( tmpDbl - floor(tmpDbl + 0.5) ) >= 0.000000001 && rightOfDec < SIGDIGMAX ) {
		tmpDbl *= 10.000000000000;
		++rightOfDec;
	}
	//FIND THE NUMBER OF DIGITS TO THE LEFT OF THE DECIMAL
	//take the base 10 logarithm of the number and convert it to an integer.  If log is less than 0, make the digits to the right 1.
	if( fabs(dVal) < 10 )
        leftOfDec = 1;
    else
        leftOfDec = (int)log10(fabs(dVal)) + 1;  //add one because the (int) cast will round down

	return leftOfDec + rightOfDec;
} //end sigDigits

//writes a predefined number of CR-LF characters to the screen
bool clearScreen( void )
{
    system("cls");

	return true;
} //end clearScreen

//function that sums up all the elements in an INTEGER array.  Returns the sum as an integer.
int sumArray( int intArray[] )
{
	int i, sum;

	sum = intArray[0];
	for( i = 1; i < MAXDEGREE; ++i )
		sum += intArray[i];

	return sum;
}  //end sumArray

//mutliplies two complex numbers, z1 and z2, and returns their product
struct cplx cMult( struct cplx z1, struct cplx z2 )
{
	struct cplx product;

	product.re = z1.re * z2.re - z1.im * z2.im;  //real part
	product.im = z1.re * z2.im + z1.im * z2.re;  //imaginary part

	return product;
}  //end cMult

//adds or subtracts two complex numbers based in the value of 'operation'
struct cplx cAddSub( struct cplx z1, struct cplx z2, int operation )
{
	struct cplx sum;
	char ch;

	if( operation == 's' ) { // 's' means SUBTRACT
		z2.re *= -1.0;
		z2.im *= -1.0;
	}
	else if( operation != 'a' ) {  // 'a' means ADD
		cout << "\n\n\tINCORRECT ARGUMENT TO cAddSub:  \"" << operation << "\".  Press any key to continue.";
		cin.get(ch);
		exit(false);
	}
	sum.re = z1.re + z2.re;
	sum.im = z1.im + z2.im;

	return sum;
}  //end cAddSub

//raises a complex number to a power by multiplying it by itself 'power' number of times.  Power must be an integer.
struct cplx cPow( struct cplx z, int exp )
{
	int i;
	struct cplx power;
	char ch;

	power.re = 0.0;
	power.im = 0.0;
	if( exp >= 2 && fabs( exp - round(exp) ) <= INTTOL ) {
		power = cMult( z, z );
		for( i = 0; i < exp - 2; ++i )
			power = cMult( z, power );
	}
	else {
		cout.precision( sigDigits(exp) );
		cout << "\n\n\tERROR:  Invalid call to cPow.  Exponent " << exp << " must be an integer 2 or higher.  Press any key to continue. ";
		cin.get(ch);  //getch();
	}

	return power;
}  //end cPow

/*********************************************** END OF UTILITY FUNCTIONS ***********************************************************************/
