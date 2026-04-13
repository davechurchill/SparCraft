#include "../SparCraft.h"
#include "SearchExperiment.h"

int main(int argc, char *argv[])
{
    SparCraft::init();

    try
    {
        if (argc == 2)
        {
            SparCraft::SearchExperiment exp(argv[1]);
            exp.runExperiment();
        }
        else
        {
            SparCraft::System::FatalError("Please provide experiment file as only argument");
        }
    }
    catch(int e)
    {
        if (e == SparCraft::System::SPARCRAFT_FATAL_ERROR)
        {
            SparCraft::SCLog() << "\nSparCraft FatalError Exception, Shutting Down\n\n";
        }
        else
        {
            SparCraft::SCLog() << "\nUnknown Exception, Shutting Down\n\n";
        }
    }
   
    return 0;
}
