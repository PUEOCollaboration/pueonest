#ifndef _PUEO_NEST_FITS_WRITER
#define _PUEO_NEST_FITS_WRITER

class TH2; 
class TTree; 

namespace pueo
{
  namespace nest
  {
    enum class FitsMode
    {
      CREATE, //create if doesn't exist, otherwise fail 
      OVERWRITE,  //overwrite if exists
      APPEND //append if it exists 
    }; 


    class FitsWriter 
    {
      public: 
        FitsWriter(const char * filename, FitsMode mode = FitsMode::CREATE); 
        virtual ~FitsWriter(); 

        int appendTH2(const TH2* img); 
        int appendTable(TTree * t, const char * varlist = "", bool bintable = true);
        int status; 
        int numext; 
      private: 
        void *ffptr; 
    }; 

  }
}
  

#endif
