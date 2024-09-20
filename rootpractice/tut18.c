#include "TStopwatch.h"
#include "TRandom2.h"

#include <iostream>


void tut18() {
    // stopwatch object
    TStopwatch t;
    
    TRandom2 *rand = new TRandom2();
    
    double x = 0;

    for (int i=0; i<1e9; i++) {
        x += rand->Rndm();
    }
        
    std::cout << x << std::endl;
    // print how long the program took
    t.Print();
}
