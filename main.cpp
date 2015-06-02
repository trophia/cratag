/**
 * Code for processing New Zealand rock lobster tag release-recapture data
 * for input into stock assessments.
 *
 * Please see the associated README.md file
 */

#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <cmath>

enum tailwidthmethod {T=1,C=2};

#warning "Date functions need to be implemented"
// These date functions originally used a date class (with `Year`, `Month` etc member)
// These are currently not reimplemented and simply return 0 which will obviously produce
// nonsense output

int DateToPeriod (int date)
{
    return 0;
    #if 0
    int year = date.Year;
    int month = date.Month;
    int season;
    if(month<=3) season = 0;
    else if(month>=4 && month<=9) season = 1;
    else season = 2;
    return (year-1945)*2 + season;
    #endif
}

int DateToCalendarYear(int date)
{
    return 0;
    //return date.Year;
}

int DateToFishingYear(int date)
{
    return 0;
    //return(date.Month<=3)?(date.Year-1):(date.Year);
}


int PeriodToFishingYear(int period)
{
    return 1945+std::floor((period-1)/2);
}
     
int AreaToCRA(int area)
{
   //CRA area for each statistical area
   int areas[43] = {1,1,1,1,2,2,2,2,3,3,3,4,4,4,4,5,5,5,5,7,7,8,8,8,8,8,8,8,9,9,9,5,5,4,9,9,9,9,1,6,6,6,6};
   if(area>=901 && area<=943) return areas[area-901];
   else return 0;
}

double CarapaceLengthToTailWidth(double cl, int sex, int cra)
{
   //Coefficients for each CRA area males then females
   //Values from Breen spreadsheet - averagae of coefficients used for CRA6 and CRA7
   double as[18] = {3.83,0.78,2.77,-0.15,2.15,3,3,6.34,5.81,-7.19,-8.03,-16.0,-12.53,-16.04,-12,-12,-14.78,-13.72};
   double bs[18] = {0.52067,0.53752,0.50577,0.53615,0.50852,0.5,0.5,0.48261,0.48464,0.68593,0.7052,0.80111,0.76186,0.81402,0.75,0.75,0.76493,0.76936};

   int index = (sex-1)*9+cra-1;
   return as[index] + bs[index]*cl;
}

double TailWidthToCarapaceLength(double tw, int sex, int cra)
{
   //Coefficients for each CRA area males then females
   //Values from Breen spreadsheet - averagae of coefficients used for CRA6 and CRA7
   double as[18] = {3.83,0.78,2.77,-0.15,2.15,3,3,6.34,5.81,-7.19,-8.03,-16.0,-12.53,-16.04,-12,-12,-14.78,-13.72};
   double bs[18] = {0.52067,0.53752,0.50577,0.53615,0.50852,0.5,0.5,0.48261,0.48464,0.68593,0.7052,0.80111,0.76186,0.81402,0.75,0.75,0.76493,0.76936};

   int index = (sex-1)*9+cra-1;
   return (tw-as[index])/ bs[index];
}


//Data types
class Record {
public:
   //Attributes recorded at each time a tagged lobster is recorded (release or recapture)
   std::string Project;
   std::string Type;
   std::string Tag;
   std::string ID;
   int Event;

   int Source; //From inititial release or a subsequent recapture

   int Date;
   int Count; //Number of observation of this individual.  Initial release = 0

   int Sex;
   int Stage;

   double CarapaceLength;
   double TailWidth;

   int Condition;
   tailwidthmethod TailWidthMethod;

   int Area;
   double Lat;
   double Lon;

   float Depth;
   float Bath;

   //Pointer to this records associated release
   Record* Recapture;

   //Constructors
      //Default
   Record():
      Date(0),
      TailWidthMethod(T),
      Event(0),
      Count(0),
      Source(0),
      Recapture(nullptr)
   {}

   //Input
   //From an output file from the Ministry of Fisheries tag database
   std::ifstream& Read(std::ifstream& file)
   {
      //Some temporary fields for conversion
      int area;
      double lat, lon;
      std::string stage, stagemethod, lonhemi;

      #warning "Date format in input file not currently defined"

      //Read in attributes
      file>>Project>>Type>>Tag>>Date;
      file>>area>>lat>>lon>>lonhemi;
      file>>Depth;
      file>>Sex;
      file>>CarapaceLength>>TailWidth>>stagemethod>>stage;
      file>>Condition>>Source;

      //Create a unique ID
      ID = Project + Type + Tag;

      //Check valid area
      if(area>=901 && area<=943) Area = area;
      else Area = NAN;

      //Convert lat and long from degrees and decimal minutes to decimal degrees
      if(std::isfinite(lat)){
         //Appropriate divisors depend on number of digits
         double div1;
         if(lat>=1e8)div1=1e7;
         else if(lat>=1e7)div1=1e6;
         else if(lat>=1e6)div1=1e5;
         else if(lat>=1e5)div1=1e4;
         else if(lat>=1e4)div1=1e3;
         else if(lat>=1e3)div1=1e2;
         else if(lat>=1e2)div1=1e2;
         else if(lat>=1e1)div1=1e1;
         //Remove first two digits as degrees
         double latdeg = floor(lat/div1);
         //Decimalised remainder and add
         Lat = -(latdeg + (lat-latdeg*div1)/(div1*0.6));
      }
      else Lat = NAN;

      if(std::isfinite(lon)){
         //Appropriate divisors depend on number of digits
         double div1;
         if(lon>=1e8)div1=1e6; //!Assumes lon>100
         else if(lon>=1e7)div1=1e5;
         else if(lon>=1e6)div1=1e4;
         else if(lon>=1e5)div1=1e3;
         else if(lon>=1e4)div1=1e2;
         else if(lon>=1e3)div1=1e1;
         else if(lon>=1e2)div1=1e0;
         //Remove first two digits as degrees
         double londeg = floor(lon/div1);
         //Decimalised remainder and add
         Lon = londeg + (lon-londeg*div1)/(div1*0.6);
         //Set hemisphere if W , note that missing values for lonhemi are assumed to be east
         if(lonhemi == "W") Lon = -Lon;
      }
      else Lon = NAN;

      //Convert stage to a consistent Stage code dependent on StageMethod
      if(Sex == 1) Stage = 1;
      else if(stage == "IF" || stage == "2") Stage = 2;
      else if(stage == "MF" || stage == "3") Stage = 3;
      else if(stage == "BF" || stage == "4") Stage = 4;
      else if(stage == "5") Stage = 5;
      else if(stage == "6") Stage = 6;
      else if(stage == "7") Stage = 7;
      else Stage = NAN; //!Non valid stages get made Missing

      return file;
   }

   //Error checking
   bool CheckSize(void)
   {
      if(not std::isfinite(TailWidth) || TailWidth<20 || TailWidth>150)
         return false;
      else
         return true;
   }

   int Consistent(const Record& recap)
   {
      //Change in sex
      if(Sex != recap.Sex)return 1;
      //Checks on change in size only done if size recorded at release and recapture
      if(std::isfinite(TailWidth) and std::isfinite(recap.TailWidth)){
         //Impossibly large shrinkage in tail width
         if((recap.TailWidth - TailWidth) < -10) return 2;
         //Impossibly large increase in tail width
         if((recap.TailWidth - TailWidth) > 40) return 3;
      }
      return 0;
   }

   //Output
   std::ofstream& Write(std::ofstream& file) const
   {
      file<<Event<<"\t";
      file<<ID<<"\t";
      file<<Project<<"\t";
      file<<Type<<"\t";
      file<<Sex<<"\t";
      file<<Date<<"\t";
      file<<DateToCalendarYear(Date)<<"\t";
      file<<DateToFishingYear(Date)<<"\t";
      file<<DateToPeriod(Date)<<"\t";
      file<<Stage<<"\t";
      file<<Condition<<"\t";
      file<<TailWidth<<"\t";
      file<<TailWidthMethod<<"\t";
      file<<Area<<"\t";
      file<<Lat<<"\t";
      file<<Lon;
      return file;
   }

   std::ofstream& LibertyWrite(std::ofstream& file)
   {
      if(Recapture != 0){
         file<<Event<<"\t";
         file<<ID<<"\t";
         file<<Project<<"\t";
         file<<Type<<"\t";
         file<<Sex<<"\t";
         int dayslib = (Recapture->Date)-Date;
         file<<Date<<"\t"<<Recapture->Date<<"\t"<<dayslib<<"\t";
         file<<DateToPeriod(Date)<<"\t"<<DateToPeriod(Recapture->Date)<<"\t";
         file<<Count<<"\t"<<(Recapture->Count)<<"\t";
         file<<Stage<<"\t";
         file<<(Recapture->Stage)<<"\t";
         file<<Condition<<"\t";
         file<<(Recapture->Condition)<<"\t";
         file<<TailWidth<<"\t";
         file<<TailWidthMethod<<"\t";
         file<<(Recapture->TailWidth)<<"\t";
         file<<(Recapture->TailWidthMethod)<<"\t";
         file<<Area<<"\t";
         file<<(Recapture->Area)<<"\t";
         file<<Depth<<"\t";
         file<<(Recapture->Depth)<<"\t";
         file<<Lat<<"\t";
         file<<Lon<<"\t";
         file<<Bath<<"\t";
         file<<(Recapture->Lat)<<"\t";
         file<<(Recapture->Lon)<<"\t";
         file<<(Recapture->Bath)<<"\n";

         // The following code calculates a bearing and distance travelled
         // The `Distance` and `Bearing` functions were from an external library
         // They could be replicated fairly easily but are curently not because these values are not
         // used for the assessment
         #if 0
         if(Lat!=Missing && Lon!=Missing
            && (Recapture->Lat!=Missing) && (Recapture->Lon!=Missing)){
               file<<Distance(Lat,Lon,Recapture->Lat,Recapture->Lon)<<"\t";
               file<<Bearing(Lat,Lon,Recapture->Lat,Recapture->Lon)<<"\n";
         }
         else
            file<<"NA"<<"\t"<<"NA"<<"\n";
         #endif
      }
      return file;
   }

   //Whether a record is valid for 2000 assessment model
   bool lob00Valid(int cra)
   {
      if(AreaToCRA(Area)==cra && CheckSize() && Recapture != 0)
         if(Recapture->CheckSize() && DateToPeriod(Date)!=DateToPeriod(Recapture->Date))
            return true;
      return false;
   }

   //Whether a record is valid for 2001 assessment model
   bool lob01Valid(int cra)
   {
      if(AreaToCRA(Area)==cra && CheckSize() && Recapture != 0)
         if(Recapture->CheckSize())
            return true;
      return false;
   }
   
   //Whether a record is valid for 2002 NSN assessment model
   bool lob02Valid(void)
   {
      if((AreaToCRA(Area)==1 || AreaToCRA(Area)==2) && CheckSize() && Recapture != 0)
         if(Recapture->CheckSize())
            return true;
      return false;
   }

   //Whether a record is valid for a particular CRA
   bool lob02bValid(int cra)
   {
      return lob00Valid(cra);//Used lob00Valid because only want to have those tags released and recaptured in the same period.
   }

   //Writes in the lob00 model format to a file given a cra area
   std::ofstream& lob00Write(std::ofstream& file, int cra)
   {
      if(lob00Valid(cra)){
         file<<Sex<<"\t"
            <<TailWidth<<"\t"
            <<(Recapture->TailWidth)<<"\t"
            <<DateToPeriod(Date)<<"\t"
            <<DateToPeriod(Recapture->Date)<<"\n";
      }
      return file;
   }

   //Writes in the lob01 model format to a file given a cra area
   std::ofstream& lob01Write(std::ofstream& file,std::map<std::string,int>& typekey, int cra)
   {
    if(lob01Valid(cra)){
            file<<Event<<"\t"
               <<Sex<<"\t"
               <<DateToPeriod(Date)<<"\t"
               <<DateToPeriod(Recapture->Date)<<"\t"
               <<TailWidth<<"\t"
               <<(Recapture->TailWidth)<<"\t"
               <<Count<<"\t"
               <<Area<<"\t"
               <<Condition<<"\t"
               <<typekey[Type]<<"\t"
               <<1<<"\t" //Dummy column
               <<"\n";
      }
      return file;
   }


   std::ofstream& lob02Write(std::ofstream& file,std::map<std::string,int>& typekey)
   {
            file<<Event<<"\t"
               <<Sex<<"\t"
               <<DateToPeriod(Date)<<"\t"
               <<DateToPeriod(Recapture->Date)<<"\t"
               <<TailWidth<<"\t"
               <<(Recapture->TailWidth)<<"\t"
               <<Count<<"\t"
               <<Area<<"\t"
               <<Condition<<"\t"
               <<typekey[Type]<<"\t"
               <<1<<"\t" //Dummy column
               <<"\n";
      return file;
   }

   std::ofstream& lob02bWrite(std::ofstream& file,std::map<std::string,int>& typekey)
   {
            file<<Event<<"\t"
               <<Sex<<"\t"
               <<DateToPeriod(Date)<<"\t"
               <<DateToPeriod(Recapture->Date)<<"\t"
               <<TailWidth<<"\t"
               <<(Recapture->TailWidth)<<"\t"
               <<Count<<"\t"
               <<Area<<"\t"
               <<Condition<<"\t"
               <<typekey[Type]<<"\t"
               <<(Recapture->Area)<<"\t" //Dummy column is now recapture area
               <<"\n";
      return file;
   }

   //Comparison operator so that can sort records in a set
   bool operator<(const Record& r) const
   {
      //Sort by ID and then date
      if(ID<r.ID) return true;
      else if(ID==r.ID){
         if(Date<r.Date) return true;
         else return false;
      }
      else return false;
   }

};//class Record

class Records: public std::vector<Record>
{
public:
   //A list of records that are to be excluded and a code for why excluded
   std::map<std::string,int> Excludes;
   //A key to numeric codes for tag types
   std::map<std::string,int> TypeKey;
   //Number of valid release-recapture pairs
   int PairsNum;
   //Number of unique IDs
   int Unique;

   //Read from file
   void Read(const std::string& filename)
   {
        std::ifstream file(filename);
        while(file.good()){
            Record record;
            record.Read(file);
        }
   }

   //Create a numeric for a combination of ID and Date and create key
   //for tag type
   void AssignCodes(void)
   {
      int event = 1;
      //Loop through all records...
      for(iterator i=begin();i!=end();i++){
         //..give event number and increment
         i->Event = event++;
         //..add tag type to key
         if(TypeKey.find(i->Type)==TypeKey.end())
            TypeKey[i->Type] = TypeKey.size() + 1;
      }
   }

   //Consistency check - all observations for a tag must be
   //consistent otherwise the tag is excluded
   void Consistency(void)
   {
      Unique = 1;
      //For each record...
      for(iterator curr=begin();curr!=end();curr++){
         iterator next = curr;next++;
         //..if the next ID is the same as the current ID
         if(next!=end()){
         if(next->ID == curr->ID){
            //..update the tag release count number
            next->Count = curr->Count + 1;
            //..if next is not consistent then add this tag to the exclude list
            int reason = curr->Consistent(*next);
            if(reason>0) Excludes[curr->ID] = reason;
         }
         else Unique++; //Otherwise increment unique ID count
         }
      }
   }

   //Finding recapture
   void Recaptures(void)
   {
      PairsNum = 0;
      //Loop through all records...
      for(iterator curr=begin();curr!=end();curr++){
         //..if this record is not in the exclude list
         if(Excludes.find(curr->ID)==Excludes.end()){
            iterator next = curr; next++;
            //..if the next tag is the same as the current one
            if(next!=end()){
            if(next->ID == curr->ID){
                  //..make it the recapture
                  curr->Recapture = &(*next);
                  PairsNum++;
            }}
         }
         //..otherwise make the recapture for this record null
         else curr->Recapture = 0;
      }
   }

   void TailWidths(void)
   {
      //If carapace length is available then convert to tail width using area
      //of initial release. Area of inital release is used to prevent different
      //value for converions parameters impacting on increment

      //For each record...
      for(iterator curr=begin();curr!=end();curr++){
         //..if it is the initial release...
         if(curr->Count == 0){
            //..record the area of inital release..
            int area = curr->Area;
            //..and for this and each subsequent record of that tag...
            iterator next = curr;//Note that starts off with this same record
            while(curr->ID == next->ID){
               //..if tail width is missing but CL and sex are not..
               if(std::isfinite(next->TailWidth) and std::isfinite(next->CarapaceLength)){
                  //..estimate tail width based on initial release area
                  next->TailWidth = CarapaceLengthToTailWidth
                        (next->CarapaceLength,next->Sex,AreaToCRA(area));
                  next->TailWidthMethod = C;
               }
               //..then go to next record
               next++;
               if(next == end()) break;
            }
         }

         //Add 0.5mm to all tail width measurements done at recaptures post-1992
         //(introduction of logbook programme when participants were told to round down)
         if(curr->TailWidthMethod==T and DateToCalendarYear(curr->Date)>1992 && curr->Source==2)
            curr->TailWidth = curr->TailWidth + 0.5;
      }
   }

   void Process(void)
   {
      AssignCodes();
      TailWidths();
      Consistency();
      Recaptures();
   }

   void ReleasesWrite(std::ofstream& file)
   {
      file<<"Event\tID\tProject\tTagType\tSex\tDateRel\tYearRel\tFYRel\tPeriodRel\tStageRel\t"
         <<"CondRel\tTWRel\tTWMethRel\tAreaRel\tLatRel\tLonRel\n";
      for(iterator i=begin();i!=end();i++){
          if(i->Count == 0){
               i->Write(file);
               file<<"\n";
          }
      }
   }

   void LibertyWrite(std::ofstream& file)
   {
      file<<"#Event\tID\tProject\tTagType\tSex\tDateRel\tDateRec\tDaysLib\tPeriodRel\t"
         <<"PeriodRec\tCountRel\tCountRec\tStageRel\tStageRec\tCondRel\tCondRec\t"
         <<"TWRel\tTWMethRel\tTWRec\tTWMethRec\tAreaRel\tAreaRec\tDepthRel\tDepthRec\t"
         <<"LatRel\tLonRel\tBathRel\tLatRec\tLonRec\tBathRec\tDistance\tBearing\n";
      for(iterator i=begin();i!=end();i++) i->LibertyWrite(file);
   }

   int lob00Number(int cra)
   {
      int number = 0;
      for(iterator i=begin();i!=end();i++)
         if(i->lob00Valid(cra))
            number++;
      return number;
   }

   int lob01Number(int cra)
   {
      int number = 0;
      for(iterator i=begin();i!=end();i++)
         if(i->lob01Valid(cra))
            number++;
      return number;
   }
   
   int lob02Number(void)
   {
      int number = 0;
      for(iterator i=begin();i!=end();i++)
         if(i->lob02Valid())
            number++;
      return number;
   }

   int lob02bNumber(int cra)
   {
      int number = 0;
      for(iterator i=begin();i!=end();i++)
         if(i->lob02bValid(cra))
            number++;
      return number;
   }

   void lob00Write(std::ofstream& file, int cra)
   {
      //Header
      file<<"#CRA"<<cra<<" tag release-recapures.\n";
      //Number of rows
      file<<"#Number\n"<<lob00Number(cra)<<"\n";
      //Recaptures
      file<<"#Sex\tTWRel\tTWRec\tPeriodRel\tPeriodRec\n";
      for(iterator i=begin();i!=end();i++) i->lob00Write(file,cra);
      //Test code - repeat area number 4 time
      file<<"#Test\n"<<cra<<cra<<cra<<cra<<"\n";
   }

   void lob01Write(std::ofstream& file, int cra, int max=1e6)
   {
      //Header
      file<<"#CRA"<<cra<<" tag release-recapures.\n";
      //Number of rows
      int records = lob01Number(cra);
      int number = records<max?records:max;
      file<<"#Number\n"<<number<<"\n";
      //Recaptures
      file<<"#Event\tSex\tPeriodRel\tPeriodRec\tTWRel\tTWRec\tRelease\tArea\tCondition\tType\tDummy\n";
      int count=0;
      for(iterator i=begin();i!=end() && count<number;i++){
       if(i->lob01Valid(cra)){
         i->lob01Write(file,TypeKey,cra);
         count++;
       }
      }
      //Test code - repeat area number 4 times
      file<<"#Test\n"<<cra<<cra<<cra<<cra<<"\n";
   }
   
   void lob02Write(std::ofstream& file)
   {
      //Header
      file<<"#CRA 1 & 2 tag release-recapures.\n";
      //Number of rows
      int number = lob02Number();
      file<<"#Number\n"<<number<<"\n";
      //Recaptures
      file<<"#Event\tSex\tPeriodRel\tPeriodRec\tTWRel\tTWRec\tRelease\tArea\tCondition\tType\tDummy\n";
      int count=0;
      for(iterator i=begin();i!=end() && count<number;i++){
       if(i->lob02Valid()){
         i->lob02Write(file,TypeKey);
         count++;
       }
      }
      //Test code
      file<<"#Test\n121212\n";
   }

   void lob02bWrite(std::ofstream& file, int cra)
   {
      //Header
      file<<"#CRA"<<cra<<"tag release-recapures.\n";
      //Number of rows
      int number = lob02bNumber(cra);
      file<<"#Number\n"<<number<<"\n";
      //Recaptures
      file<<"#Event\tSex\tPeriodRel\tPeriodRec\tTWRel\tTWRec\tRelease\tArea\tCondition\tType\tDummy\n";
      int count=0;
      for(iterator i=begin();i!=end() && count<number;i++){
       if(i->lob02bValid(cra)){
         i->lob02bWrite(file,TypeKey);
         count++;
       }
      }
      //Test code
      file<<"#Test\n"<<cra<<cra<<cra<<cra<<"\n";
   }

};//class Records

int main(int argc, char* argv[]){
    Records tags;

    //Read from data file
    std::cout<<"Reading tags\n";
    tags.Read("Records.txt");

    //Assign event number and tag type key
    std::cout<<"AssignCodes\n";
    tags.AssignCodes();

    //Process the tags
    std::cout<<"Processing tags\n";
    tags.Process();

    //Ouput inital releases
    std::cout<<"Releases output\n";
    std::ofstream releases("releases.dat");
    tags.ReleasesWrite(releases);

    //Output to lob file
    std::cout<<"lob output\n";
    std::ofstream lobDat("tags.dat");
    tags.lob02Write(lobDat);
    
    //Output excludes
    std::cout<<"Excludes output\n";
    std::ofstream excludes("excludes.dat");
    for(auto pair : tags.Excludes) excludes<<pair.first<<"\t"<<pair.second<<"\n";

    //Output tag types key
    std::cout<<"Tag type keys\n";
    std::ofstream tagkey("tagkey.out");
    for(auto pair : tags.TypeKey) tagkey<<pair.first<<"\t"<<pair.second<<"\n";

    return 0;
}
