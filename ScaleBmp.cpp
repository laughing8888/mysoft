#include <stdio.h>
#include <Windows.h>

typedef struct _ScaleBMP ScaleBMP;
struct _ScaleBMP
{
  unsigned char *pBmpInBuf;//读入图像数据的指针
  unsigned char *pBmpOutBuf;//保存图像的指针
  int bmpWidth;//图像的宽
  int bmpHeight;//图像的高
  RGBQUAD *pColorTable;//颜色表指针
  int biBitCount;//图像类型，每像素位数

  int bmpOutWidth;///out width
  int bmpOutHeight;///out width
};

bool writeBMP(char *bmpName, ScaleBMP *scaleBmp, RGBQUAD *pColorTable)
{
  if(!scaleBmp->pBmpOutBuf) 
    return 0;
  int colorTablesize = 0;  //颜色表大小，以字节为单位，灰度图像颜色表为1024字节
  if(scaleBmp->biBitCount == 8)
    colorTablesize = 1024;
  int closlineByte=(scaleBmp->bmpOutWidth * scaleBmp->biBitCount / 8 + 3)/4*4;
  FILE *fp=fopen(bmpName,"wb");    //以二进制写的方式打开文件
  if(fp==0) return 0;
  BITMAPFILEHEADER fileHead;   //申请位图文件头结构变量，填写文件头信息
  fileHead.bfType = 0x4D42;//bmp类型
  fileHead.bfSize= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)+ colorTablesize + closlineByte*scaleBmp->bmpOutHeight;//bfSize是图像文件4个组成部分之和
  fileHead.bfReserved1 = 0;
  fileHead.bfReserved2 = 0;
  fileHead.bfOffBits=54+colorTablesize;//bfOffBits是图像文件前3个部分所需空间之和
  fwrite(&fileHead, sizeof(BITMAPFILEHEADER),1, fp);//写文件头进文件
  BITMAPINFOHEADER head; //申请位图信息头结构变量，填写信息头信息
  head.biBitCount=scaleBmp->biBitCount;
  head.biClrImportant=0;
  head.biClrUsed=0;
  head.biCompression=0;
  head.biHeight=scaleBmp->bmpOutHeight;
  head.biPlanes=1;
  head.biSize=40;
  head.biSizeImage=closlineByte*head.biHeight;
  head.biWidth=scaleBmp->bmpOutWidth;
  head.biXPelsPerMeter=0;
  head.biYPelsPerMeter=0;   
  fwrite(&head, sizeof(BITMAPINFOHEADER), 1, fp); //写位图信息头进内存
  if(pColorTable)
    fwrite(pColorTable,sizeof(RGBQUAD),256,fp);  //灰度图，将其颜色表写入文件
  fwrite(scaleBmp->pBmpOutBuf, scaleBmp->bmpOutHeight*closlineByte, 1, fp);   //写位图数据进文件
#ifdef DEBUG_FILE
  FILE *file = fopen("outBMP.txt", "wb");
  fwrite(scaleBmp->pBmpOutBuf, 1, closlineByte*scaleBmp->bmpHeight, file);
  fclose(file);
#endif
  fclose(fp); //关闭文件
  return 1;
}

bool readBmp(char *bmpName, ScaleBMP *scaleBmp)
{
  FILE *fp=fopen(bmpName, "rb");
  if(fp==0) return 0;
  BITMAPFILEHEADER filehead;
  fread(&filehead, sizeof(filehead), 1, fp);
  fseek(fp, sizeof(BITMAPFILEHEADER),0);//跳过位图文件头结构BITMAPFILEHEADER  
  BITMAPINFOHEADER head;  //定义位图信息头结构变量，读取位图信息头进内存，存放在变量head中
  fread(&head, sizeof(BITMAPINFOHEADER), 1,fp); //获取图像宽、高、每像素所占位数等信息
  scaleBmp->bmpWidth = head.biWidth;
  scaleBmp->bmpHeight = head.biHeight;
  scaleBmp->biBitCount = head.biBitCount;//定义变量，计算图像每行像素所占的字节数（必须是4的倍数）
  int openlineByte=(scaleBmp->bmpWidth * scaleBmp->biBitCount/8+3)/4*4;
  if(scaleBmp->biBitCount==8)
  {
    scaleBmp->pColorTable=new RGBQUAD[256];              //申请颜色表所需要的空间
    fread(scaleBmp->pColorTable,sizeof(RGBQUAD),256,fp);//读颜色进内存空间
  }
  scaleBmp->pBmpInBuf = (unsigned char *)malloc(openlineByte * scaleBmp->bmpHeight);
  int size = fread(scaleBmp->pBmpInBuf,1,openlineByte * scaleBmp->bmpHeight,fp);//申请位图数据所需要的空间，读位图数据进内存
#ifdef DEBUG_FILE
  FILE *file = fopen("inBMP.txt", "wb");
  fwrite(scaleBmp->pBmpInBuf, 1, openlineByte*scaleBmp->bmpHeight, file);
  fclose(file);
#endif
  fclose(fp);//关闭文件
  return 1;//读取文件成功
}

bool ScaleBmp(ScaleBMP *scaleBmp, float wScale, float hScale, bool tag)
{
  int outWidth = scaleBmp->bmpWidth * wScale + 0.5;
  int outHeight = scaleBmp->bmpHeight * hScale + 0.5;
  scaleBmp->bmpOutWidth = outWidth;
  scaleBmp->bmpOutHeight = outHeight;

  int i,j,m,countliner = 0;
  m = scaleBmp->biBitCount / 8;
  int count255 = 0;
  int lineByte = (outWidth * scaleBmp->biBitCount / 8 + 3) / 4 * 4;
  scaleBmp->pBmpOutBuf = (unsigned char*)malloc(lineByte * outHeight);
  printf("color data byte = %d\n", lineByte*outHeight);
  for( i = 0; i < scaleBmp->bmpOutHeight; ++i)
    for( j = 0; j < scaleBmp->bmpOutWidth; ++j)
    { 
      if(!tag)//线性插值
      {
        int inPosX = i / hScale + 0.5;
        int inPosY = j / wScale + 0.5;
        if(inPosX >= 0 && inPosX < scaleBmp->bmpHeight &&
          inPosY >= 0 && inPosY < scaleBmp->bmpWidth)
        {
          ++countliner;
          for(int k = 0; k < m; ++k)
            if(scaleBmp->biBitCount == 24)
              scaleBmp->pBmpOutBuf[(i*3*outWidth + j*3) + k] = 
                scaleBmp->pBmpInBuf[(inPosX*3*scaleBmp->bmpWidth + inPosY*3) + k];
            else if(scaleBmp->biBitCount == 8)
              scaleBmp->pBmpOutBuf[(i*outWidth + j) + k] = 
              scaleBmp->pBmpInBuf[(inPosX*scaleBmp->bmpWidth + inPosY) + k];
        }
        else
        {
          ++count255;
          for(int k = 0; k < m; ++k)
            if(scaleBmp->biBitCount == 24)
              scaleBmp->pBmpOutBuf[i*3*outWidth + j*3 + k] = 255;
            else if(scaleBmp->biBitCount == 8)
              scaleBmp->pBmpOutBuf[i*outWidth + j + k] = 255;
        }
      }
      else///双线性插值
      {
        float inPosX = (i + 0.5) / hScale - 0.5;
        float inPosY = (j + 0.5) / wScale - 0.5;
        float X = inPosX - int(inPosX);
        float Y = inPosY - int(inPosY);
        int oldX = int(inPosX);
        int oldY = int(inPosY);
        int upleft = oldX*scaleBmp->bmpWidth + oldY;///左上角
        int downleft = (oldX+1 >= scaleBmp->bmpHeight ? scaleBmp->bmpHeight-1:oldX+1)*scaleBmp->bmpWidth + oldY;///左下角
        for(int k = 0; k < m; ++k)
          if(scaleBmp->biBitCount == 24)
            scaleBmp->pBmpOutBuf[i*3*outWidth + j*3 + k] = 
              scaleBmp->pBmpInBuf[upleft*3 + k]*(1-X)*(1-Y) +
              scaleBmp->pBmpInBuf[(upleft+1)*3 + k]*(1-Y)*X +
              scaleBmp->pBmpInBuf[downleft*3 + k]*(1-X)*Y +
              scaleBmp->pBmpInBuf[(downleft+1)*3 + k]*X*Y;
          else if(scaleBmp->biBitCount == 8)
            scaleBmp->pBmpOutBuf[(i)*outWidth + j + k] = 
            scaleBmp->pBmpInBuf[upleft + k]*(1-X)*(1-Y) + 
            scaleBmp->pBmpInBuf[upleft +1+ k]*(1-Y)*X + 
            scaleBmp->pBmpInBuf[downleft + k]*(1-X)*Y + 
            scaleBmp->pBmpInBuf[downleft+1 + k]*X*Y;
      }
    }
    printf("x=%d y=%d countliner=%d count255=%d totalbyte=%d\n", \
            i,j,countliner,count255,lineByte * outHeight);
    return true;

  return false;
}


int main(int argn, char *argc[])
{
  if( argn >= 4 )
  {
    int tag = 1;
    char *pBmpPath;
    float widthScale;
    float heightScale;
    if(argn == 4)
    {
      pBmpPath = argc[argn - 3];
      widthScale = atof(argc[argn - 2]);
      heightScale = atof(argc[argn - 1]);
    }
    else if(argn == 5)
    {
      pBmpPath = argc[argn - 4];
      widthScale = atof(argc[argn - 3]);
      heightScale = atof(argc[argn - 2]);
      tag = atoi(argc[argn - 1]);
    }
    else
      return 0;

    ScaleBMP scaleBmp;
    if(readBmp(pBmpPath, &scaleBmp))
    {
      if(ScaleBmp(&scaleBmp, widthScale, heightScale, tag))
      {
        if(scaleBmp.biBitCount == 24)
          writeBMP("scale.bmp", &scaleBmp, 0);
        else if(scaleBmp.biBitCount == 8)
          writeBMP("scale.bmp", &scaleBmp, scaleBmp.pColorTable);
        free(scaleBmp.pBmpInBuf);
        free(scaleBmp.pBmpOutBuf);
      }
    }
  }
  return 0;
}