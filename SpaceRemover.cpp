/* This program is required because some of the tag numbers have spaces in them so thave 
to be removed first. 
*/
#include <fstream.h>
#include <string>
int main(void)
{
   //Make a input file stream from the first argument
   ifstream input("Records.txt");

   //Make a output file stream from the second argument
   ofstream output("RecordsNew.txt");
  
  char chr;
  while(input.get(chr))
    if(chr!=' ') output.put(chr);

   return 0;
}

