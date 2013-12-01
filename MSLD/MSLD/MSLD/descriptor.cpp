//descriptor.cpp
#pragma  once
#include "stdafx.h"
#include "descriptor.h"
#include "wzhlib.h"
#include <math.h>


SCNo	LUTSubRegion[360][nMaxRegionNum*nEachRPixes];	//�������ڵĵ�
SCNo	LUTDes2Region[nMaxRegionNum*3-2];				//����������
SCPos	LUTBiPos[nMaxRegionNum*nEachRPixes];
double	LUTWeight[nMaxRegionNum*nEachRPixes];

/********************************************************************************
					����	����
					����	����
********************************************************************************/
CDescriptor::CDescriptor(double* pGrayData,int nWidth,int nHegiht,double* pLinePts,int inLineCounts,int szPtsCounts[])
{
	m_nLineCount = inLineCounts;

	//�����ܸ���
	m_nTotolPts = 0;
	for(int i = 0; i < inLineCounts; i++)
	{
		m_nTotolPts = m_nTotolPts + szPtsCounts[i];
		m_szPtsCounts[i] = szPtsCounts[i];
	}

	//ͼ��
	m_pImageData = NULL;
	if(pGrayData != NULL)
	{
		m_nWidth		= nWidth;
		m_nHeight		= nHegiht;
		m_nTotolPixels	= m_nWidth*m_nHeight;
		m_pImageData	= new double[m_nTotolPixels];
		memcpy(m_pImageData, pGrayData, sizeof(double)*m_nTotolPixels);
		m_pLinePts		= new double[m_nTotolPts*2];
		memcpy(m_pLinePts,pLinePts,sizeof(double)*m_nTotolPts*2);		
		m_pMainArc		= new double[m_nLineCount];
		wzhSet(m_pMainArc,0.0f,m_nLineCount);
	}

	//�ݶ�ͼ��
	m_pDxImage	 = new double[m_nTotolPixels];
	m_pDyImage	 = new double[m_nTotolPixels];
	m_pMagImage	 = new double[m_nTotolPixels];

	//����
	m_fSigma	= 1.2f;
	m_scDes		= NULL;

	//���������
	m_nDesDim				= nDesDim;
	m_scDes					= new float[m_nDesDim*m_nLineCount];
	m_pByValidFlag			= new byte[inLineCounts];
	memset(m_pByValidFlag,0,sizeof(byte)*inLineCounts);

	//��ʼ�����ұ�
	InitializeLUT();
}

CDescriptor::~CDescriptor()
{
	//ͼ��
	wzhFreePointer(m_pImageData);

	//ֱ�ߵ�
	wzhFreePointer(m_pLinePts);
	wzhFreePointer(m_pByValidFlag);

	//�ݶ�ͼ��
	wzhFreePointer(m_pDxImage);
	wzhFreePointer(m_pDyImage);
	wzhFreePointer(m_pMagImage);

	//������
	wzhFreePointer(m_scDes);
	wzhFreePointer(m_pMainArc);
}

/********************************************************************************
				��ʼ�����ұ�
********************************************************************************/
void  CDescriptor::InitializeLUT()
{
	//�ȼ���0�Ƚǵĵ�
	int nC = nMaxRegionNum*nH/2;
	for(int i = 0;i<nMaxRegionNum; i++)
		for(int j = 0; j < nH; j++)
			for(int k=0; k < nW; k++)
			{
				int temp = nW*nH*i + j*nW +k;
				LUTSubRegion[0][temp].nNo1 = k-(nW-1)/2;
				LUTSubRegion[0][temp].nNo2 = i*nH + j - nC;
			}
	
	//�������תλ��
	for(int i= 1;i < 360; i++)
	{	
		double dArc = -(double)(i*PI/180);
		for(int j= 0; j < nMaxRegionNum*nEachRPixes; j++)
			{
				int xx = LUTSubRegion[0][j].nNo1;
				int yy = LUTSubRegion[0][j].nNo2;
				LUTSubRegion[i][j].nNo1 = wzhRound(xx*cos(dArc) - yy*sin(dArc));
				LUTSubRegion[i][j].nNo2 = wzhRound(xx*sin(dArc) + yy*cos(dArc));
			}
	}

	//�����
	int nDim = 0;
	for(int i = 0; i < nMaxRegionNum; i++)
	{
		LUTDes2Region[nDim].nNo1 = i;
		LUTDes2Region[nDim].nNo2 = i;
		nDim ++;
	}

	//�����
	for(int i = 0; i < nMaxRegionNum-1; i++)
	{
		LUTDes2Region[nDim].nNo1 = i;
		LUTDes2Region[nDim].nNo2 = i+1;
		nDim ++;
		LUTDes2Region[nDim].nNo1 = i+1;
		LUTDes2Region[nDim].nNo2 = i;
		nDim ++;
	}

	//
	double dSigma = 22.0;
	int nR = (nH-1)/2;
	for(int i=0; i<nMaxRegionNum; i++)
		for(int j=0; j<nH; j++)
			for(int k=0; k<nW;k++)
			{
				int P = i*nEachRPixes + j*nW + k;
				int nNo1 = 0;
				int nNo2 = 0;
				double dCoe1 = 0;
				if(j < nR)
				{
					nNo1 = i-1;
					nNo2 = nNo1 + 1;
					dCoe1 = double(nR-j)/nH;
				}
				else if(j == nR)
				{
					nNo1 = i;
					nNo2 = i;
					dCoe1 = 1;
				}
				else
				{
					nNo1 = i;
					nNo2 = nNo1 + 1;
					dCoe1 = 1 - double(j-nR)/nH;
				}
				
				//����
				if(nNo1 == -1)
				{
					nNo1 = 0;
					dCoe1 = 1;
				}
				if(nNo2 == nMaxRegionNum)
				{
					nNo2	= nMaxRegionNum-1;
					dCoe1	= 0;
				}

				LUTBiPos[P].nNo1	= nNo1;
				LUTBiPos[P].nNo2	= nNo2;
				LUTBiPos[P].dCoe1	= dCoe1;
				LUTBiPos[P].dCoe2	= 1-dCoe1;

				int nC		 = (nH*nMaxRegionNum-1)/2;
				double	d	 = (double)abs(i*nH+j-nC);
				LUTWeight[P] = exp(-d*d/(2*dSigma*dSigma));
			}
}

/********************************************************************************
					ֱ��������
********************************************************************************/
void  CDescriptor::ComputeLineDescriptor()
{
	//�����ݶ�ͼ��
	ConputeGaussianGrad(m_pDxImage,m_pImageData,m_nWidth,m_nHeight,m_fSigma,11);
	ConputeGaussianGrad(m_pDyImage,m_pImageData,m_nWidth,m_nHeight,m_fSigma,12);
	ComputeMag(m_pMagImage,m_pDxImage,m_pDyImage,m_nTotolPixels);

	//����ÿһ��ֱ�ߵ�������
	int nPtsPos = 0;
	double* pSubDesLineDes	= new double[nMaxRegionNum*8];
	for(int nNo = 0; nNo < m_nLineCount;nNo++)
	{
		//*************************************************
		//		1 ���ֱ������Ч�ĵ㳬��һ��,����Ч
		//*************************************************
		int nPtsPos_bak = nPtsPos;
		int nInValid	= 0;
		double dDxAvg	= 0;
		double dDyAvg	= 0;
		for(int nT1=0; nT1<m_szPtsCounts[nNo]; nT1++)
		{
			//��õ�ǰ���λ����Ϣ
			int	nCenterR	= (int)m_pLinePts[2*nPtsPos];
			int	nCenterC	= (int)m_pLinePts[2*nPtsPos+1];
			int nCenterP	= nCenterR*m_nWidth + nCenterC;
			dDxAvg			= dDxAvg + m_pDxImage[nCenterP];
			dDyAvg			= dDyAvg + m_pDyImage[nCenterP];
			nPtsPos++;

			//�ж��Ƿ�Խ��
			if(	nCenterR < SCRadius+1 || nCenterR > m_nHeight-SCRadius-1 || 
				nCenterC < SCRadius+1 || nCenterC > m_nWidth-SCRadius-1)
			{
				nInValid ++;
			}
		}
		int nValid = m_szPtsCounts[nNo] - nInValid;
		if(nInValid > m_szPtsCounts[nNo]/2)
		{
			m_pByValidFlag[nNo] = 0;
			continue;
		}
		else
		{
			m_pByValidFlag[nNo] = 1;
		}
		
		//***************************************************
		//		2	����ֱ�ߵ�������
		//***************************************************
		double dMainArc = 0;
		if(matchType == 1)
			dMainArc = ComputeLineDir(&m_pLinePts[2*nPtsPos_bak],m_szPtsCounts[nNo],dDxAvg,dDyAvg);

		//***************************************************
		//		3	���������������Ӿ���
		//***************************************************
		int nReCount = 0;
		nPtsPos = nPtsPos_bak;
		double* pSubDesMatrix  = new double[nMaxRegionNum*4*nValid];
		wzhSet(pSubDesMatrix,0,nMaxRegionNum*4*nValid);
		for(int nT1=0; nT1<m_szPtsCounts[nNo]; nT1++)
		{
			//��õ�ǰ���λ����Ϣ
			int	nCenterR	= (int)m_pLinePts[2*nPtsPos];
			int	nCenterC	= (int)m_pLinePts[2*nPtsPos+1];
			int nCenterP	= nCenterR*m_nWidth + nCenterC;
			nPtsPos++;

			//�ж��Ƿ�Խ��
			if(	nCenterR < SCRadius+1 || nCenterR > m_nHeight-SCRadius-1 || 
				nCenterC < SCRadius+1 || nCenterC > m_nWidth-SCRadius-1)
			{
				continue;
			}

			//��ǰ�ݶ���Ϣ
			if(matchType != 1)
			{
				dMainArc = atan2(-m_pDyImage[nCenterP],-m_pDxImage[nCenterP]);
				dMainArc = LimitArc(dMainArc);
			}

			//����ֱ�ߵ������Ӿ����
			double pSingleSubDes[nMaxRegionNum*4];
			ComputeSubRegionProjection(pSingleSubDes,dMainArc,nCenterR,nCenterC);
			memcpy(&pSubDesMatrix[nMaxRegionNum*4*nReCount],pSingleSubDes,sizeof(double)*nMaxRegionNum*4);
			nReCount++;
		}

		//***************************************************
		//		4	���������Ӳ�����������
		//***************************************************
		ComputeDescriptorByMatrix(pSubDesLineDes,pSubDesMatrix,nMaxRegionNum*4,nValid);
		for(int g = 0; g < nMaxRegionNum*8; g++)
		{
			m_scDes[nNo*m_nDesDim+g] = (float)pSubDesLineDes[g];
		}

		//***************************************************
		//		5	�ͷ��ڴ�
		//***************************************************
		wzhFreePointer(pSubDesMatrix);
	}
	wzhFreePointer(pSubDesLineDes);
}

/********************************************************************************
							����ֱ�ߵ�������
********************************************************************************/
void  CDescriptor::ComputeSubRegionProjection(double* pSubRegionDes,double dMainArc,int nCenterR,int nCenterC)
{
	//ȡ��9��С�����ڵĵ��ݶ�
	int nMainAngle = (int)(dMainArc*180/PI);
	double* pDataDx = new double[nMaxRegionNum*nEachRPixes];
	double* pDataDy = new double[nMaxRegionNum*nEachRPixes];
	for(int i=0; i<nMaxRegionNum; i++)
		for(int j=0; j<nEachRPixes; j++)
		{
			int k = i*nEachRPixes + j;
			int rr = LUTSubRegion[nMainAngle][k].nNo1 + nCenterR;
			int cc = LUTSubRegion[nMainAngle][k].nNo2 + nCenterC;
			int kk = rr*m_nWidth+cc;
			
			if(kk < 0 || kk > m_nTotolPixels-1)
			{
				continue;
			}
			pDataDx[k] = m_pDxImage[kk];
			pDataDy[k] = m_pDyImage[kk];
		}

	//������
	double dLineVx = cos(dMainArc);
	double dLineVy = sin(dMainArc);

	//����ÿһ����ĸ�����
	for(int i=0; i< 4*nMaxRegionNum; i++)
	{
		pSubRegionDes[i] = 0;
	}
	for(int i=0; i<nMaxRegionNum*nEachRPixes; i++)
	{
		//�ݶȼ�Ȩ
		double dx = pDataDx[i]*LUTWeight[i];
		double dy = pDataDy[i]*LUTWeight[i];
		double IP = dx*dLineVx + dy*dLineVy;
		double EP = dx*dLineVy - dy*dLineVx;

		//�������ӽ���2�������Ӧ��Ȩֵ
		int nNo1 = LUTBiPos[i].nNo1;
		int nNo2 = LUTBiPos[i].nNo2;
		double dCoe1 = LUTBiPos[i].dCoe1;
		double dCoe2 = LUTBiPos[i].dCoe2;
		
		//�ۼӵ�����1��
		if(IP > 0)
		{
			pSubRegionDes[4*nNo1]	 = pSubRegionDes[4*nNo1] + IP*dCoe1;
		}
		else
		{
			pSubRegionDes[4*nNo1+2]	 = pSubRegionDes[4*nNo1+2] + abs(IP*dCoe1);
		}
		if(EP > 0)
		{
			pSubRegionDes[4*nNo1+1]	 = pSubRegionDes[4*nNo1+1] + EP*dCoe1;
		}
		else
		{
			pSubRegionDes[4*nNo1+3]	 = pSubRegionDes[4*nNo1+3] + abs(EP*dCoe1);
		}

		//�ۼӵ�����2��
		if(IP > 0)
		{
			pSubRegionDes[4*nNo2]	 = pSubRegionDes[4*nNo2] + IP*dCoe2;
		}
		else
		{
			pSubRegionDes[4*nNo2+2]	 = pSubRegionDes[4*nNo2+2] + abs(IP*dCoe2);
		}
		if(EP > 0)
		{
			pSubRegionDes[4*nNo2+1]	 = pSubRegionDes[4*nNo2+1] + EP*dCoe2;
		}
		else
		{
			pSubRegionDes[4*nNo2+3]	 = pSubRegionDes[4*nNo2+3] + abs(EP*dCoe2);
		}

	}
	/***********************************************************************/
	//�ͷ��ڴ�
	wzhFreePointer(pDataDx);
	wzhFreePointer(pDataDy);
}

/********************************************************************************
���������Ӿ������������
********************************************************************************/
void  CDescriptor::ComputeDescriptorByMatrix(double* pLineDes,double* pMatrix,int nD,int nValid)
{
	//�����ֵ
	double* pAvg = new double[nD];
	wzhSet(pAvg,0,nD);
	for(int i = 0; i < nD; i++)
	{
		for(int j = 0; j < nValid; j++)
		{
			int k = j*nD + i;
			pAvg[i] = pAvg[i] + pMatrix[k];
		}
	}
	for(int i = 0; i < nD; i++)
	{
		pAvg[i] = pAvg[i]/nValid;
	}

	//�����׼��
	double* pStd = new double[nD];
	wzhSet(pStd,0,nD);
	for(int i = 0; i < nD; i++)
	{
		for(int j = 0; j < nValid; j++)
		{
			int k = j*nD + i;
			double dVar = (pMatrix[k]-pAvg[i])*(pMatrix[k]-pAvg[i]);
			pStd[i] = pStd[i] + dVar;
		}
	}
	for(int i = 0; i < nD; i++)
	{
		pStd[i] = sqrt(pStd[i]/nValid);
	}

	//�ֱ��һ��
	wzhNormorlizeNorm(pAvg,nD);
	wzhNormorlizeNorm(pStd,nD);

	//������
	for(int i = 0; i < nD; i++)
	{
		if(pAvg[i] < 0.4)
			pLineDes[i]		= pAvg[i];
		else
			pLineDes[i]		= 0.4;
		if(pStd[i] < 0.4)
			pLineDes[nD+i]	= pStd[i];
		else
			pLineDes[nD+i]	= 0.4;
	}

	wzhNormorlizeNorm(pLineDes,2*nD);

	//�ͷ��ڴ�
	wzhFreePointer(pStd);
	wzhFreePointer(pAvg);
}

double  CDescriptor::ComputeLineDir(double* pLinePts,int nCount,double dDxAvg, double dDyAvg)
{
	//������С���˼�������
	initM(MATCOM_VERSION);
	Mm mMatrix = zeros(nCount,3);
	for(int g1 = 0; g1 < nCount; g1++)
	{
		mMatrix.r(g1+1,1) = pLinePts[2*g1];
		mMatrix.r(g1+1,2) = pLinePts[2*g1+1];
		mMatrix.r(g1+1,3) = 1;
	}

	//����ֵ�ֽ��þ�ȷλ��
	Mm u,s,v;
	i_o_t i_o = {0,0};
	svd(mMatrix,i_o,u,s,v);

	//���㷽��
	double a = v.r(1,3);
	double b = v.r(2,3);
	double dMainArc = atan2(-b,a);
	dMainArc = LimitArc(dMainArc);

	//�˳�
	exitM();

	//�ж�����
	double dMainArc1 = dMainArc - PI/2;
	dMainArc1 = LimitArc(dMainArc1);
	double dMainArc2 = dMainArc + PI/2;
	dMainArc2 = LimitArc(dMainArc2);
	double dAvgArc = atan2(-dDyAvg,-dDxAvg);
	dAvgArc = LimitArc(dAvgArc);

	double error1 = ArcDis(dMainArc1,dAvgArc);
	double error2 = ArcDis(dMainArc2,dAvgArc);

	//�������շ���
	double nArcReturn = dMainArc1;
	if(error1 > error2)
		nArcReturn = dMainArc2;

	return nArcReturn;
}