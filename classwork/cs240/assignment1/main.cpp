# {{{ includes
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
# }}}

using namespace std;

# {{{ printData
void printData ( int j, float k ) {
    cout << fixed << setprecision(1) << "  " << char(j+65) << "\t" << k << "%\t| ";
    for ( int i=0.0; i < k; i++ ) {
        cout << "*";
    }
    cout << endl;
}
# }}}

int main() {
# {{{ initialization
    int count = 0, letters[26] = {0};
    char Ch;

    ifstream inStream;
    inStream.open( "file.txt" );
# }}}

# {{{ content
    if ( !inStream ) {
        cerr << "Unable to open file.txt -- Do you have read permissions?";
        exit(1);
    }

    while (!inStream.eof()) {
        inStream >> Ch;
        Ch = toupper(Ch);
        // upper ranges are 65-90
        if ( (int(Ch) >= 65) && (int(Ch) <= 90) ) {
            letters[int(Ch)-65]++;
            count++;
        }
    }

    float percent = count/100.0;
    cout << "Char\tPerc\t|" << endl;
    cout << "----------------------------------" << endl;
    for ( int j=0; j < 26; j++ ) {
         printData(j, letters[j]/percent);
    }
# }}}

# {{{ closing time
    inStream.close();
    return 0;
# }}}
}
