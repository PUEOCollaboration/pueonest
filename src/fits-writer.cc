#include "TH2.h" 
#include <iostream>
#include <cstdlib> 
#include <cfitsio/fitsio.h> 
#include "pueo/nest/fits-writer.hh"

const int MAX_NAME_LEN=255; 



#define CHECK_STATUS \
if (status != 0) {\
  fprintf(stderr,"line %d", __LINE__);\
  fits_report_error(stderr, status);\
  fits_close_file(fptr, &status);\
  return status;\
}
 
#define CHECK_STATUS_RETVOID \
if (status != 0) {\
  fprintf(stderr,"line %d", __LINE__);\
  fits_report_error(stderr, status);\
  fits_close_file(fptr, &status);\
  return;\
}


pueo::nest::FitsWriter::FitsWriter(const char * fitsfilename, FitsMode mode) 
{
  ffptr = 0;
  fitsfile * fptr = (fitsfile*) ffptr; 

  status=0; 

  //attempt to open the file 
  //does the file already exist? 
  
  if (!access(fitsfilename, R_OK))
  {
    // the file is there already, check mode to see whta to do
    // yes there's a race condition here, but it's not so easy to fix... 
    if (mode == FitsMode::OVERWRITE) 
    {
      unlink(fitsfilename); 
    }
    else if (mode == FitsMode::CREATE) 
    {
      fprintf(stderr,"File %s already exists and opened with FM_CREATE\n", fitsfilename); 
      status = -1; 
      return; 
    }
    else
    {
      fits_open_file(&fptr, fitsfilename, READWRITE, &status);
      ffptr = fptr; 
      CHECK_STATUS_RETVOID; 
      fits_get_num_hdus(fptr, &numext, &status); 
      CHECK_STATUS_RETVOID; 
      numext--; //first one isn't an extension
      return; 
    }
  }

  if (!fptr)
  {
    printf("Creating %s\n\n", fitsfilename); 
    fits_create_file(&fptr, fitsfilename, &status); 
    ffptr = fptr; 
    std::cout << fptr << std::endl; 
    CHECK_STATUS_RETVOID
    numext = 0; 
  }


  // Create the primary HDU (empty image)
  int naxis=2; // Number of axes
  long naxes[2] = {0, 0}; //Array containing length of each axis
  fits_create_img(fptr, FLOAT_IMG, naxis, naxes, &status);
  CHECK_STATUS_RETVOID
  
  const char * telescope = "PUEO"; 
  // Set telescope name in header using keyword TELESCOP
  // fits_write_key(file_pointer,data_type,keyword_name,keyword_value,comment,returned_status)
  fits_write_key(fptr,TSTRING,"TELESCOP",(void*) telescope,"Telescope name",&status);
  CHECK_STATUS_RETVOID

}
  
pueo::nest::FitsWriter::~FitsWriter() 
{
  // Close the FITS file
  fitsfile * fptr = (fitsfile*) ffptr; 
  if (fptr) 
  {
    fits_close_file(fptr, &status);
    CHECK_STATUS_RETVOID
  }

}

template <typename htype, typename dtype>
static void * filldata (htype * h) 
{
  int nx = h->GetNbinsX();
  int ny = h->GetNbinsY();
  dtype * v = h->GetArray(); 

  dtype * data = (dtype*) malloc(sizeof(dtype) * nx * ny);

  for (int iy = 0; iy < ny; iy++) 
  {
    memcpy( data + iy * nx, v + (iy+1) * (nx+2) + 1, nx * sizeof(dtype)); 
  }
  return (void*) data; 

}

struct imgdata
{
  
  int fits_imgtype; 
  int fits_type; 
  long naxes[2]; 
  double xmin, ymin;
  double dx, dy;
  std::string name; 
  std::string title; 
  void * data;
  const TH2 * h; 

  imgdata(const TH2 * img) 
  {
    h = img; 
    naxes[0] = h->GetNbinsX();
    naxes[1] = h->GetNbinsY();
    xmin = h->GetXaxis()->GetXmin();
    ymin = h->GetYaxis()->GetXmin();
    dx = h->GetXaxis()->GetBinWidth(1);
    dy = h->GetYaxis()->GetBinWidth(1);
    name = h->GetName(); 
    title = h->GetTitle(); 

    if (img->InheritsFrom("TArrayC"))  
    {
      fits_imgtype = BYTE_IMG; 
      fits_type = TBYTE; 
      data = filldata<TH2C,char>((TH2C*) h); 
    }
    else if (img->InheritsFrom("TArrayS"))
    {
      fits_imgtype = SHORT_IMG; 
      fits_type = TSHORT; 
      data = filldata<TH2S,int16_t>((TH2S*) h); 
    }
    else if (img->InheritsFrom("TArrayI")) 
    {
      fits_imgtype=LONG_IMG; 
      fits_type = TLONG; 
      data = filldata<TH2I,int32_t>((TH2I*) h); 
    }
    else if (img->InheritsFrom("TArrayF"))
    {
      fits_imgtype = FLOAT_IMG;
      fits_type = TFLOAT; 

      data = filldata<TH2F,float>((TH2F*) h); 
    }
    else if (img->InheritsFrom("TArrayD"))
    {
      fits_imgtype = DOUBLE_IMG;
      fits_type = TDOUBLE; 
      data = filldata<TH2D,double>((TH2D*) h); 
    }
    else
    {
      std::cerr<< "Invalid hist type doesn't inherit from usable TArray!\n" << std::endl; 
      fits_imgtype = 0; 
      fits_type = 0; 
      data = 0; 
    }
  }

  ~imgdata() 
  {
    if (data) free (data); 
  }
}; 




int pueo::nest::FitsWriter::appendTH2(const TH2 *img) 
{

  fitsfile * fptr = (fitsfile*) ffptr; 
  long npixels; //Total number of pixels in an image

  int iimg = 0; 
  fits_get_num_hdus(fptr, &iimg, &status); 
  CHECK_STATUS
  imgdata id(img);
        
  
  fits_create_img(fptr, id.fits_imgtype, 2, id.naxes, &status);
  CHECK_STATUS

  // Write values to image
  long fpixel=1; //First pixel to use
  npixels = id.naxes[0]* id.naxes[1]; //Calculate total number of pixels
  fits_write_img(fptr, id.fits_type, fpixel, npixels, id.data, &status);
  CHECK_STATUS


  numext++; 
  char extname[MAX_NAME_LEN]; //Name of extension
  // Create name for extension
  sprintf(extname,"EXTENSION%d",numext);
  // Set extension name by writing keyword in header
  // fits_write_key(file_pointer,data_type,keyword_name,keyword_value,comment,returned_status)
  fits_write_key(fptr,TSTRING,"EXTNAME",extname,"Extension name",&status);
  CHECK_STATUS
  
  // Set telescope name in header using keyword TELESCOP
  // fits_write_key(file_pointer,data_type,keyword_name,keyword_value,comment,returned_status)
  const char * telescope = "PUEO"; 
  fits_write_key(fptr,TSTRING,"TELESCOP",(void*) telescope,"Telescope name",&status);
  CHECK_STATUS
  
  // Example of writing a keyword that has integer value
  // fits_write_key(file_pointer,data_type,keyword_name,keyword_value,comment,returned_status)
  fits_write_key(fptr,TINT,"EXTNUM",&numext,"Extension number",&status);
  CHECK_STATUS

  unsigned short p = 1; 
  fits_write_key(fptr, TUSHORT, "CRPIX1",&p, "X reference pixel", &status); 
  CHECK_STATUS
  fits_write_key(fptr, TUSHORT, "CRPIX2",&p, "Y reference pixel", &status); 
  CHECK_STATUS
  float ref = img->GetXaxis()->GetXmin(); 
  fits_write_key(fptr, TFLOAT, "CRVAL1", &ref, "X reference value ", &status); 
  CHECK_STATUS
  ref = img->GetYaxis()->GetXmin(); 
  fits_write_key(fptr, TFLOAT, "CRVAL2", &ref, "Y reference value ", &status ); 
  CHECK_STATUS
  float bwidth = img->GetXaxis()->GetBinWidth(1); 
  fits_write_key(fptr, TFLOAT, "CDELT1", &bwidth,"X-axis increment [deg]", &status); 
  CHECK_STATUS
  bwidth = img->GetYaxis()->GetBinWidth(1); 
  fits_write_key(fptr, TFLOAT, "CDELT2", &bwidth,"Y-axis increment [deg]", &status); 
  CHECK_STATUS
  

 return status;
}
