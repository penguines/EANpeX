# EANpeX
用于识别EAN条形码图像以及读取EAN条码信息。
Recognize and parse EAN barcode image.

EANpeX.h/EANpeX.cpp :
  用于对已获得的条形码中单行光强数据处理。无需STL以外其他库支持。
  Used to process fetched light intensity data. Work independently.
  
EANpeX_cv.h/EANpeX_cv.cpp :
  基于OpenCV，用于从图像中识别EAN条码区域，以及从EAN条码图像中读取条码信息。需要安装OpenCV。
  Based on OpenCV. Used to fetch EAN barcode area from images and parse EAN barcode image. OpenCV required.
