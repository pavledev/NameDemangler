#include <iostream>
#include "MSVCDemangler.h"

using namespace std;

int main(int argc, char* argv[])
{
    MSVCDemangler msvcDemangler = MSVCDemangler();
    bool verbose = false;

    for (int i = 1; i < argc; i++)
    {
        if (i == 1 && string(argv[i]) == "-v")
        {
            verbose = true;

            continue;
        }

        string mangledName = argv[i];
        string rest;
        string demangledName = msvcDemangler.DemangleSymbol(mangledName, rest, verbose);

        if (verbose)
        {
            cout << endl;
        }

        cout << demangledName << endl;

        if (rest.length())
        {
            cout << "Rest (" << rest << ")";
        }

        cout << endl;
    }

    cin.get();

    return 0;
}
