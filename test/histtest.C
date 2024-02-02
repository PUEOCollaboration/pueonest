R__LOAD_LIBRARY(build/libpueonest.so)
#include "pueo/nest/fits-writer.hh" 
void histtest() 
{

  pueo::nest::FitsWriter f("histtest.fit",pueo::nest::FitsMode::OVERWRITE); 

  TH2I h2("test","Test hist",100,-5,5,100,-5,5); 

  for (int i = 0; i < 10000; i++) h2.Fill(gRandom->Gaus(), gRandom->Gaus()); 

  f.appendTH2(&h2); 


}
