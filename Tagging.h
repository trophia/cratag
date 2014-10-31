#ifndef _TaggingH
#define _TaggingH

#include <Symtax.h>
using namespace Symtax;
#include <NZCRA.h>
using namespace NZCRA;
#include "Lobster.h"

enum tailwidthmethod {T=1,C=2};

namespace Tagging
{

//Data types
class Record: public Named{
public:
   //Attributes recorded at each time a tagged lobster is recorded (release or recapture)
   String Project;
   String Type;
   String Tag;
   String ID;
   int Event;

   int Source; //From inititial release or a subsequent recapture

   Date Date;
   int Count; //Number of observation of this individual.  Initial release = 0

   int Sex;
   Number<int> Stage;

   Number<double> CarapaceLength;
   Number<double> TailWidth;

   Number<int> Condition;
   tailwidthmethod TailWidthMethod;

   Number<int> Area;
   Number<double> Lat;
   Number<double> Lon;

   Number<float> Depth;
   float Bath;

   //Pointer to this records associated release
   Record* Recapture;

   //Constructors
      //Default
   Record():
      Date(0,0,0),
      TailWidthMethod(T),
      Event(0),
      Count(0),
      Source(0),
      Recapture(0)
   {}
   //From a lobster
   Record(Symtax::Date& date, Lobster::Indiv& lobster, double tailwidth, int source):
      Project("IBM"),
      Type("Lob"),
      Tag(IntToString(lobster.ID)),
      ID("IBMLob"+IntToString(lobster.ID)),
      Date(date),
      Sex(lobster.Sex),
      Stage(lobster.Mature),
      TailWidth(tailwidth),
      TailWidthMethod(T),
      Event(0),
      Count(0),
      Source(source),
      Recapture(0),
      Area(lobster.Area)
   {}

   //Input
      //From an output file from the Ministry of Fisheries tag database
   InputFile& Read(InputFile& file)
   {
      //Some temporary fields for conversion
      Number<int> area;
      Number<double> lat, lon;
      String stage, stagemethod, lonhemi;

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
      else Area = Missing;

      //Convert lat and long from degrees and decimal minutes to decimal degrees
      if(lat!=Missing){
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
      else Lat = Missing;

      if(lon!=Missing){
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
      else Lon = Missing;

      //Convert stage to a consistent Stage code dependent on StageMethod
      if(Sex == 1) Stage = 1;
      else if(stage == "IF" || stage == "2") Stage = 2; //! This conversion needs to be checked
      else if(stage == "MF" || stage == "3") Stage = 3;
      else if(stage == "BF" || stage == "4") Stage = 4;
      else if(stage == "5") Stage = 5;
      else if(stage == "6") Stage = 6;
      else if(stage == "7") Stage = 7;
      else Stage = Missing; //!Non valid stages get made Missing

      return file;
   }

   //Error checking
   bool CheckSize(void)
   {
      if(TailWidth == Missing || TailWidth<20 || TailWidth>150)
         return false;
      else
         return true;
   }

   int Consistent(const Record& recap)
   {
      //Change in sex
      if(Sex != recap.Sex)return 1;
      //Checks on change in size only done if size recorded at release and recapture
      if(TailWidth != Missing && recap.TailWidth != Missing){
         //Impossibly large shrinkage in tail width
         if((recap.TailWidth - TailWidth) < -10) return 2;
         //Impossibly large increase in tail width
         if((recap.TailWidth - TailWidth) > 40) return 3;
      }
      return 0;
   }

   //Output
   OutputFile& Write(OutputFile& file) const
   {
      file<<Event<<tab;
      file<<ID<<tab;
      file<<Project<<tab;
      file<<Type<<tab;
      file<<Sex<<tab;
      file<<Date<<tab;
      file<<Date.Year<<tab;
      file<<DateToFishingYear(Date)<<tab;
      file<<DateToPeriod(Date)<<tab;
      file<<Stage<<tab;
      file<<Condition<<tab;
      file<<TailWidth<<tab;
      file<<TailWidthMethod<<tab;
      file<<Area<<tab;
      file<<Lat<<tab;
      file<<Lon;
      return file;
   }

   OutputFile& LibertyWrite(OutputFile& file)
   {
      if(Recapture != 0){
         file<<Event<<tab;
         file<<ID<<tab;
         file<<Project<<tab;
         file<<Type<<tab;
         file<<Sex<<tab;
         int dayslib = (Recapture->Date)-Date;
         file<<Date<<tab<<Recapture->Date<<tab<<dayslib<<tab;
         file<<DateToPeriod(Date)<<tab<<DateToPeriod(Recapture->Date)<<tab;
         file<<Count<<tab<<(Recapture->Count)<<tab;
         file<<Stage<<tab;
         file<<(Recapture->Stage)<<tab;
         file<<Condition<<tab;
         file<<(Recapture->Condition)<<tab;
         file<<TailWidth<<tab;
         file<<TailWidthMethod<<tab;
         file<<(Recapture->TailWidth)<<tab;
         file<<(Recapture->TailWidthMethod)<<tab;
         file<<Area<<tab;
         file<<(Recapture->Area)<<tab;
         file<<Depth<<tab;
         file<<(Recapture->Depth)<<tab;
         file<<Lat<<tab;
         file<<Lon<<tab;
         file<<Bath<<tab;
         file<<(Recapture->Lat)<<tab;
         file<<(Recapture->Lon)<<tab;
         file<<(Recapture->Bath)<<tab;
         if(Lat!=Missing && Lon!=Missing
            && (Recapture->Lat!=Missing) && (Recapture->Lon!=Missing)){
               file<<Distance(Lat,Lon,Recapture->Lat,Recapture->Lon)<<tab;
               file<<Bearing(Lat,Lon,Recapture->Lat,Recapture->Lon)<<endl;
         }
         else
            file<<"NA"<<tab<<"NA"<<endl;
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
   OutputFile& lob00Write(OutputFile& file, int cra)
   {
      if(lob00Valid(cra)){
         file<<Sex<<tab
            <<TailWidth<<tab
            <<(Recapture->TailWidth)<<tab
            <<DateToPeriod(Date)<<tab
            <<DateToPeriod(Recapture->Date)<<endl;
      }
      return file;
   }

   //Writes in the lob01 model format to a file given a cra area
   OutputFile& lob01Write(OutputFile& file,Table<String,int>& typekey, int cra)
   {
	if(lob01Valid(cra)){
            file<<Event<<tab
               <<Sex<<tab
               <<DateToPeriod(Date)<<tab
               <<DateToPeriod(Recapture->Date)<<tab
               <<TailWidth<<tab
               <<(Recapture->TailWidth)<<tab
               <<Count<<tab
               <<Area<<tab
               <<Condition<<tab
               <<typekey[Type]<<tab
               <<1<<tab //Dummy column
               <<endl;
      }
      return file;
   }


   OutputFile& lob02Write(OutputFile& file,Table<String,int>& typekey)
   {
            file<<Event<<tab
               <<Sex<<tab
               <<DateToPeriod(Date)<<tab
               <<DateToPeriod(Recapture->Date)<<tab
               <<TailWidth<<tab
               <<(Recapture->TailWidth)<<tab
               <<Count<<tab
               <<Area<<tab
               <<Condition<<tab
               <<typekey[Type]<<tab
               <<1<<tab //Dummy column
               <<endl;
      return file;
   }

   OutputFile& lob02bWrite(OutputFile& file,Table<String,int>& typekey)
   {
            file<<Event<<tab
               <<Sex<<tab
               <<DateToPeriod(Date)<<tab
               <<DateToPeriod(Recapture->Date)<<tab
               <<TailWidth<<tab
               <<(Recapture->TailWidth)<<tab
               <<Count<<tab
               <<Area<<tab
               <<Condition<<tab
               <<typekey[Type]<<tab
               <<(Recapture->Area)<<tab //Dummy column is now recapture area
               <<endl;
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

class Records: public Set<Record>
{
public:
   //A list of records that are to be excluded and a code for why excluded
   Table<String,int> Excludes;
   //A key to numeric codes for tag types
   Table<String,int> TypeKey;
   //Number of valid release-recapture pairs
   int PairsNum;
   //Number of unique IDs
   int Unique;

   //Constructor
   Records(const char* name)
      :Set<Record>(name),
      Excludes("Excludes"),
      TypeKey("TypeKey")
   {}

   Records(const char* name, String filename)
      :Set<Record>(name),
      Excludes("Excludes"),
      TypeKey("TypeKey")
   {
      Date::Format = "dd/mm/yyyy";
      InputFile file(filename);
      file.Labels = false;
      file>>*this;
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
         if(!TypeKey.Present(i->Type))
            TypeKey[i->Type] = TypeKey.Size() + 1;
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
         if(Excludes.Present(curr->ID)==false){
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
               if(next->TailWidth==Missing && next->CarapaceLength!=Missing){
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
         if(curr->TailWidthMethod==T && curr->Date.Year>1992 && curr->Source==2)
            curr->TailWidth = curr->TailWidth + 0.5;
      }
   }

   void PosCheckBath(void)
   {
      //An array of 5 min by 5 min cells around NZ specifying depth
      //160E to 185e (175W) = 25*12 = 300 cells
      //-30S to -55S = 25 * 12 = 300 cells
      float bath[300][300];

      //Read in bathymettry data and classify cells as valid or not
      InputFile bathy("../Data/Tag/nzbath.dat");
      double lat,lon,depth;
      cout<<"Classifying cells based on bathymettry\n";
      while(bathy>>lon>>lat>>depth){
         //Calculate cell number
         int x = floor((lon-160)/0.0833);
         int y = floor((-lat-30)/0.0833);
         if(x>=0 && x<300 && y>=0 && y<300)
            bath[x][y] = depth;
      }

      cout<<"Checking records for valid position\n";
      for(iterator i=begin();i!=end();i++){
         if(i->Lon!=Missing && i->Lat!=Missing){
            //Calculate cell number
            int x;
            if(i->Lon>0)//eastern hemisphere
               x = floor((i->Lon-160)/0.0833);
            else//western hemisphere
               x = floor((i->Lon+360-160)/0.0833);
            int y = floor((-(i->Lat)-30)/0.0833);
            //If in range of cells then check record depth
            if(x>=0 && x<300 && y>=0 && y<300)
               i->Bath = bath[x][y];
            else
               i->Bath = -4000;
         }
         else
            i->Bath = -4000;
      }
   }

   void PosCheckPoly(void)
   {
      cout<<"Reading in polygons\n";
      PolygonSet coast;
      InputFile coastdat("../Data/Tag/nzcoast.dat");
      coast.Read(coastdat);
      PolygonSet d200;
      InputFile d200dat("../Data/Tag/nz200.dat");
      d200.Read(d200dat);

      cout<<"Position checking using polygons\n";
      for(iterator i=begin();i!=end();i++){
        i->Bath = -1;
        if(i->Lon!=Missing && i->Lat!=Missing){
          //Check if it is in any of the <200m polygons with 2nm buffer
          for(int p=0;p<d200.Polygons.GetSize() && i->Bath != 0;p++)
            if(d200.Polygons[p].Contains(Point(i->Lon,i->Lat),2.0/60.0))
              i->Bath = 0;
          //Check if on land with 0.5nm buffer
          for(int p=0;p<coast.Polygons.GetSize() && i->Bath != 1;p++)
            if(coast.Polygons[p].Contains(Point(i->Lon,i->Lat),-0.25/60.0))
              i->Bath = 1;
        }
      }
   }

   void Process(void)
   {
      AssignCodes();
      TailWidths();
      Consistency();
      //PosCheckPoly();
      Recaptures();
   }

   void ReleasesWrite(OutputFile& file)
   {
      file.Labels = false;
      file<<"Event\tID\tProject\tTagType\tSex\tDateRel\tYearRel\tFYRel\tPeriodRel\tStageRel\t"
         <<"CondRel\tTWRel\tTWMethRel\tAreaRel\tLatRel\tLonRel\n";
      for(iterator i=begin();i!=end();i++){
          if(i->Count == 0){
               i->Write(file);
               file<<endl;
          }
      }
   }

   void LibertyWrite(OutputFile& file)
   {
      file.Labels = false;
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

   void lob00Write(OutputFile& file, int cra)
   {
      //Header
      file<<"#CRA"<<cra<<" tag release-recapures. Produced "<<Now.AsString()<<endl;
      //Number of rows
      file<<"#Number\n"<<lob00Number(cra)<<endl;
      //Recaptures
      file<<"#Sex\tTWRel\tTWRec\tPeriodRel\tPeriodRec\n";
      for(iterator i=begin();i!=end();i++) i->lob00Write(file,cra);
      //Test code - repeat area number 4 time
      file<<"#Test\n"<<cra<<cra<<cra<<cra<<endl;
   }

   void lob01Write(OutputFile& file, int cra, int max=1e6)
   {
      //Header
      file<<"#CRA"<<cra<<" tag release-recapures. Produced "<<Now.AsString()<<endl;
      //Number of rows
      int records = lob01Number(cra);
      int number = records<max?records:max;
      file<<"#Number\n"<<number<<endl;
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
      file<<"#Test\n"<<cra<<cra<<cra<<cra<<endl;
   }
   
   void lob02Write(OutputFile& file)
   {
      //Header
      file<<"#CRA 1 & 2 tag release-recapures. Produced "<<Now.AsString()<<endl;
      //Number of rows
      int number = lob02Number();
      file<<"#Number\n"<<number<<endl;
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

   void lob02bWrite(OutputFile& file, int cra)
   {
      //Header
      file<<"#CRA"<<cra<<"tag release-recapures. Produced "<<Now.AsString()<<endl;
      //Number of rows
      int number = lob02bNumber(cra);
      file<<"#Number\n"<<number<<endl;
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

class Programme
{
public:
   //Parameters
   Index<int> Source;
   Array<DistrNorm> TWObsError;
   Array<int> Releases;
   Array<int> Recaptures;

   bool ReleaseLegals;
   Param<> RecordRate;
   bool RereleaseLegals;
   bool RecordNonLegals;

   int RecordsLimit; //The number of tagging records that this will be limited to

   //Data
   Records Tags;

   //Constructor
   Programme(void):
      Source(1,2,"Source"),
      TWObsError(Source,"TWObsError"),
      Releases("Releases"),
      ReleaseLegals(true),
      RecordRate("RecordRate"),
      RereleaseLegals(true),
      RecordsLimit(1e6),
      Tags("Tags")
   {}

   void Init(String filename, IndexCom& indexcom)
   {
      InputFile input(filename);
      input.Formats["Distr"]="Mean Stdev";
      input>>TWObsError;

      input.Formats["Param"] = "Default";
      input>>RecordRate;

      Releases.IndicesSet(indexcom);
      input>>Releases;

      Tags.Clear();
   }

   void Add(Date& date, Lobster::Indiv& lobster,int source)
   {
       //Apply obervation error
       double tw = lobster.TailWidth() + TWObsError(source).Random();
        //!Fishermen round down
        //if(source==2) tw = floor(tw);
        //Add record
        Tags.Insert(Record(date,lobster,tw,source));
   }

   void ReleaseRandomly(Lobster::Population& lobsters,const Date& date, double mintw=0)
   {
      //Release the population randomly with no handling mortality
      int tags = Releases(DateToPeriod(date));
      int tagged = 0;//Number tagged
      int failedattempts = 0;//Prevents infinite loop if no lobsters available to tag
      while(tagged<tags && failedattempts<1e4){
         Lobster::Indiv* lob = lobsters.Random();
         if(!lob->Tagged && lob->TailWidth()>=mintw){
            lob->Tagged = true;
            tagged++; failedattempts=0;
            Add(date,*lob,1);
         }
         else failedattempts++;
      }
   }

   void RecaptureAll(Lobster::Population& lobsters,const Date& date)
   {
      //Recapture all tagged animals and re-release with no handling mortality
      for(Lobster::Population::iterator lob=lobsters.begin();lob!=lobsters.end();lob++)
         if(lob->Tagged) Add(date,*lob,1);
   }//end RecaptureAll

   void RecaptureRandomly(Lobster::Population& lobsters,Date& date, double rate)
   {
      //Recapture the population randomly with no handling mortality
      int lobs = lobsters.Size() * rate;
      for(int l=1;l<=lobs;l++){
         Lobster::Population::iterator lob = lobsters.Random();
         if(lob->Tagged) Add(date,*lob,1);
      }
   }

   void Output(void)
   {
      OutputFile tags("output/tags.out");
      tags.Labels = false;
      tags<<Tags;
   }

};//end class Programme

}//end namespace Tagging

#endif
