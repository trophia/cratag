#include <Symtax.h>
#include <Tagging.h>

void Run(void)
{
   //Contruct a set of records from data file
   cout<<"Reading tags\n";
   Tagging::Records Tags("Tags","Records.txt");

   //Assign event number and tag type key
   cout<<"AssignCodes\n";
   Tags.AssignCodes();

   //Process the tags
   cout<<"Processing tags\n";
   Tags.Process();

   //Ouput inital releases
   cout<<"Releases output\n";
   OutputFile releases("releases.dat");
   Tags.ReleasesWrite(releases);

   //Output to lob file
   cout<<"lob output\n";
   OutputFile lobDat("tags.dat");
   Tags.lob02Write(lobDat);
   
   //Output excludes
   cout<<"Excludes output\n";
   OutputFile excludes("excludes.dat");
   excludes<<Tags.Excludes;

   //Output tag
   cout<<"Tag type keys\n";
   OutputFile tagkey("tagkey.out");
   tagkey<<Tags.TypeKey;
}


int main(int argc, char* argv[])
{
  Run();
  return 0;
}

 