//descriptor.h
/***************************************************************
������: CDescriptor
������	�����������������ͼ����һ���������
���룺
�����	
���ߣ�	zhwang
�ʼ���	zhwang@nlpr.ia.ac.cn
���ڣ�	06.12.30
����޸ģ�
���ԣ�	
***************************************************************/
#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include "wzhlib.h"

typedef struct SCNo
{
	int		nNo1;									// ��1�������
	int		nNo2;									// ��2�������
} SCNo;

typedef struct SCPos
{
	int		nNo1;									// ��1�������
	int		nNo2;									// ��2�������
	double	dCoe1;
	double	dCoe2;
} SCPos;

class CDescriptor
{	
	//����
	public:
		double		m_fSigma;						//��˹�˲��߶�
	public:
		//ͼ����Ϣ
		double*		m_pImageData;					//ͼ������
		int			m_nWidth;						//ͼ��߶�
		int			m_nHeight;						//ͼ����
		int			m_nTotolPixels;					//ͼ����������
		
		//�ǵ���Ϣ
		int			m_nLineCount;					//ֱ������
		int			m_nTotolPts;					//����ֱ���ϵĵ��ܸ���
		int			m_szPtsCounts[nMaxLineCount];	//����ֱ���ϵ�ĸ���
		double*		m_pLinePts;						//���λ����Ϣ
		
		//�ݶ�
		double*		m_pDxImage;						//dxͼ��
		double*		m_pDyImage;						//dyͼ��
		double*		m_pMagImage;					//�ݶȷ�ֵ
				
		//��������Ϣ
		float*		m_scDes;						//Std��������
		int			m_nDesDim;						//������ά��
		byte*		m_pByValidFlag;					//��ǽǵ��Ƿ���Ч
		double*		m_pMainArc;						//ÿ��ֱ�ߵ�������

	//������Ա
	public:
		CDescriptor(double* pGrayData,int nWidth,int nHegiht,
					double* pLinePts,int inLineCounts,int szPtsCounts[]);
		~CDescriptor();

		//����������
		void	ComputeLineDescriptor();
		
	private:
		void	InitializeLUT();
		void	ComputeDescriptorByMatrix(double* pLineDes,double* pMatrix,int nD,int nValid);
		void	ComputeSubRegionProjection(double* pDesMatrix,double dMainArc,int nCenterR,int nCenterC);

		//�����ڻ������
		double	ComputeLineDir(double* pLinePts,int nCount,double dDxAvg, double dDyAvg);
};
