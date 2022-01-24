# EANpeX
用于识别EAN条形码图像、读取EAN条码信息以及生成EAN条形码图像。

Recognize, parse and generate EAN barcode image.

EANpeX.h/EANpeX.cpp :

  用于对已获得的条形码中单行光强数据处理及条形码单行光强数据生成。无需STL以外其他库支持。
  
  Used to process fetched light intensity data and generate light intensity data of given EAN Code. Work independently.
  
EANpeX_cv.h/EANpeX_cv.cpp :

  基于OpenCV，用于从图像中识别EAN条码区域、从EAN条码图像中读取条码信息以及生成EAN条形码图像。需要安装OpenCV。
  
  Based on OpenCV. Used to fetch EAN barcode area from images and parse EAN barcode image and generate EAN Barcode image. OpenCV required.

  对于EAN条码识别提供基于霍夫线变换和轮廓提取的两种方法，前者通用性更好，能够在较为复杂的环境下进行识别。
  
  示例：
  1-
  ![1_hf](https://user-images.githubusercontent.com/47978720/150717345-7ba6d8fd-a29e-4eea-8a2d-22a72d0c62f1.jpg)

  2-
  ![result_img](https://user-images.githubusercontent.com/47978720/150717641-372f2b1c-2115-4b6f-bafd-2d4c3e377bf4.jpg)
  ![result_ean](https://user-images.githubusercontent.com/47978720/150717650-9316f0a5-10e5-47c5-a758-65d1169dc5df.jpg)
