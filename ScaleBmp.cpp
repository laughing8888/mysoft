#include <stdio.h>
#include <Windows.h>

typedef struct _ScaleBMP ScaleBMP;
struct _ScaleBMP
{
  unsigned char *pBmpInBuf;//����ͼ�����ݵ�ָ��
  unsigned char *pBmpOutBuf;//����ͼ���ָ��
  int bmpWidth;//ͼ��Ŀ�
  int bmpHeight;//ͼ��ĸ�
  RGBQUAD *pColorTable;//��ɫ��ָ��
  int biBitCount;//ͼ�����ͣ�ÿ����λ��

  int bmpOutWidth;///out width
  int bmpOutHeight;///out width
};

bool writeBMP(char *bmpName, ScaleBMP *scaleBmp, RGBQUAD *pColorTable)
{
  if(!scaleBmp->pBmpOutBuf) 
    return 0;
  int colorTablesize = 0;  //��ɫ���С�����ֽ�Ϊ��λ���Ҷ�ͼ����ɫ��Ϊ1024�ֽ�
  if(scaleBmp->biBitCount == 8)
    colorTablesize = 1024;
  int closlineByte=(scaleBmp->bmpOutWidth * scaleBmp->biBitCount / 8 + 3)/4*4;
  FILE *fp=fopen(bmpName,"wb");    //�Զ�����д�ķ�ʽ���ļ�
  if(fp==0) return 0;
  BITMAPFILEHEADER fileHead;   //����λͼ�ļ�ͷ�ṹ��������д�ļ�ͷ��Ϣ
  fileHead.bfType = 0x4D42;//bmp����
  fileHead.bfSize= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)+ colorTablesize + closlineByte*scaleBmp->bmpOutHeight;//bfSize��ͼ���ļ�4����ɲ���֮��
  fileHead.bfReserved1 = 0;
  fileHead.bfReserved2 = 0;
  fileHead.bfOffBits=54+colorTablesize;//bfOffBits��ͼ���ļ�ǰ3����������ռ�֮��
  fwrite(&fileHead, sizeof(BITMAPFILEHEADER),1, fp);//д�ļ�ͷ���ļ�
  BITMAPINFOHEADER head; //����λͼ��Ϣͷ�ṹ��������д��Ϣͷ��Ϣ
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
  fwrite(&head, sizeof(BITMAPINFOHEADER), 1, fp); //дλͼ��Ϣͷ���ڴ�
  if(pColorTable)
    fwrite(pColorTable,sizeof(RGBQUAD),256,fp);  //�Ҷ�ͼ��������ɫ��д���ļ�
  fwrite(scaleBmp->pBmpOutBuf, scaleBmp->bmpOutHeight*closlineByte, 1, fp);   //дλͼ���ݽ��ļ�
#ifdef DEBUG_FILE
  FILE *file = fopen("outBMP.txt", "wb");
  fwrite(scaleBmp->pBmpOutBuf, 1, closlineByte*scaleBmp->bmpHeight, file);
  fclose(file);
#endif
  fclose(fp); //�ر��ļ�
  return 1;
}

bool readBmp(char *bmpName, ScaleBMP *scaleBmp)
{
  FILE *fp=fopen(bmpName, "rb");
  if(fp==0) return 0;
  BITMAPFILEHEADER filehead;
  fread(&filehead, sizeof(filehead), 1, fp);
  fseek(fp, sizeof(BITMAPFILEHEADER),0);//����λͼ�ļ�ͷ�ṹBITMAPFILEHEADER  
  BITMAPINFOHEADER head;  //����λͼ��Ϣͷ�ṹ��������ȡλͼ��Ϣͷ���ڴ棬����ڱ���head��
  fread(&head, sizeof(BITMAPINFOHEADER), 1,fp); //��ȡͼ����ߡ�ÿ������ռλ������Ϣ
  scaleBmp->bmpWidth = head.biWidth;
  scaleBmp->bmpHeight = head.biHeight;
  scaleBmp->biBitCount = head.biBitCount;//�������������ͼ��ÿ��������ռ���ֽ�����������4�ı�����
  int openlineByte=(scaleBmp->bmpWidth * scaleBmp->biBitCount/8+3)/4*4;
  if(scaleBmp->biBitCount==8)
  {
    scaleBmp->pColorTable=new RGBQUAD[256];              //������ɫ������Ҫ�Ŀռ�
    fread(scaleBmp->pColorTable,sizeof(RGBQUAD),256,fp);//����ɫ���ڴ�ռ�
  }
  scaleBmp->pBmpInBuf = (unsigned char *)malloc(openlineByte * scaleBmp->bmpHeight);
  int size = fread(scaleBmp->pBmpInBuf,1,openlineByte * scaleBmp->bmpHeight,fp);//����λͼ��������Ҫ�Ŀռ䣬��λͼ���ݽ��ڴ�
#ifdef DEBUG_FILE
  FILE *file = fopen("inBMP.txt", "wb");
  fwrite(scaleBmp->pBmpInBuf, 1, openlineByte*scaleBmp->bmpHeight, file);
  fclose(file);
#endif
  fclose(fp);//�ر��ļ�
  return 1;//��ȡ�ļ��ɹ�
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
      if(!tag)//���Բ�ֵ
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
      else///˫���Բ�ֵ
      {
        float inPosX = (i + 0.5) / hScale - 0.5;
        float inPosY = (j + 0.5) / wScale - 0.5;
        float X = inPosX - int(inPosX);
        float Y = inPosY - int(inPosY);
        int oldX = int(inPosX);
        int oldY = int(inPosY);
        int upleft = oldX*scaleBmp->bmpWidth + oldY;///���Ͻ�
        int downleft = (oldX+1 >= scaleBmp->bmpHeight ? scaleBmp->bmpHeight-1:oldX+1)*scaleBmp->bmpWidth + oldY;///���½�
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