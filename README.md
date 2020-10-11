# EANpeX
用于识别EAN条形码图像、读取EAN条码信息以及生成EAN条形码图像。

Recognize, parse and generate EAN barcode image.

EANpeX.h/EANpeX.cpp :

  用于对已获得的条形码中单行光强数据处理及条形码单行光强数据生成。无需STL以外其他库支持。
  
  Used to process fetched light intensity data and generate light intensity data of given EAN Code. Work independently.
  
EANpeX_cv.h/EANpeX_cv.cpp :

  基于OpenCV，用于从图像中识别EAN条码区域、从EAN条码图像中读取条码信息以及生成EAN条形码图像。需要安装OpenCV。
  
  Based on OpenCV. Used to fetch EAN barcode area from images and parse EAN barcode image and generate EAN Barcode image. OpenCV required.
