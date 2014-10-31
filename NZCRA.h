#ifndef _NZCRAH
#define _NZCRAH
namespace NZCRA{
//A namespace for containing general functions and classes for NZ rock lobster (CRA) assessment

int DateToPeriod (Date date)
{
      int year = date.Year;
      int month = date.Month;
      int season;
      if(month<=3) season = 0;
      else if(month>=4 && month<=9) season = 1;
      else season = 2;
      return (year-1945)*2 + season;
}

int DateToFishingYear(Date date)
{
      return(date.Month<=3)?(date.Year-1):(date.Year);
}
   
int PeriodToFishingYear(int period)
{
	return 1945+floor((period-1)/2);
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


}//namespace CRA
#endif