#include "rapt.h"

//
// Update and draw loops use different random number
// generators.  Why?  If we pause the game, we can seed
// RandomDraw so that we can still call random during
// draw, but have the view unchanged.
//
Random gRandom;
Random& gRand=gRandom;



#ifdef OLDRANDOM


Random::Random(void)
{
#ifdef TRACKDOWN
	mWatcher=false;
#endif

	SetFloatPrecision();
}

void Random::Seed(int number) 
{
	if (number==-1) number=OS_Core::Tick();
    mSeed=number;

    int *piState;
    int iState;

    piState=&rgiState[2];
    piState[-2]=55-55;
    piState[-1]=55-24;
    piState[0]=(number)&((1<<30)-1);
    piState[1]=1;
    for (iState=2;iState<55;iState++) piState[iState]=(piState[iState-1]+piState[iState-2])&((1<<30)-1);
}

int Random::Get(int theMax) 
{
	if (theMax==0) return 0;

    int power;
    int number;

    for (power=2;power<theMax;power<<=1);
	number=Generate()&(power-1);
	number%=theMax;
    return number;
}

int Random::Generate() 
{
    int *piState;
    int iState1;
    int iState2;
    int iRand;

    piState=&rgiState[2];
    iState1=piState[-2];
    iState2=piState[-1];
    iRand=(piState[iState1]+piState[iState2])&((1<<30)-1);
    piState[iState1]=iRand;
    if (++iState1==55) iState1=0;
    if (++iState2==55) iState2=0;

    piState[-2]=iState1;
    piState[-1]=iState2;
    return iRand>>6;
}

#endif


