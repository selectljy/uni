#include "include.h"



									
uint16_t AD_Value[N][M]; 							//�������ADC ת�������Ҳ��DMA ��Ŀ���ַ
uint16_t After_filter[M]; 							//���������ƽ��ֵ֮��Ľ��
uint16_t DATA_BUFF[M];								//RAM���ݴ洢��--AD����ֵ

/******************************************************************************
** ��������   void GPIO_Configuration(void)
** ˵  ��:    ADC��DMA��ʼ������
** ��ڲ����� 
** ���ڲ�����  
** ��  �أ�  
** ��  ע��   
******************************************************************************/

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1,ENABLE ); //ʹ��ADC1 ͨ��ʱ�ӣ������ܽ�ʱ��
	/* Configure ADCCLK such as ADCCLK = PCLK2/6 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); 									//72M/6=12,ADC ���ʱ�䲻�ܳ���14M
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); 					//ʹ��DMA ����
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 						
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 						
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	
	//ADC ��ʼ��
	
	ADC_DeInit(ADC1); //������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ
	
	ADC_TempSensorVrefintCmd(ENABLE); 					//�����ڲ��¶ȴ�����
	
	/* ADC1 configuration �\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\�\*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; 					//ADC ����ģʽ:ADC1 ��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode =ENABLE; 						//ģ��ת��������ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 					//ģ��ת������������ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //�ⲿ����ת���ر�
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 				//ADC �����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = M;								 //˳����й���ת����ADC ͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure); 								//����ADC_InitStruct ��ָ���Ĳ�����ʼ������ADCx �ļĴ���
	/* ADC1 regular channel11 configuration */
	//����ָ��ADC �Ĺ�����ͨ�����������ǵ�ת��˳��Ͳ���ʱ��
	//ADC1,ADC ͨ��x,�������˳��ֵΪy,����ʱ��Ϊ239.5 ����
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 5, ADC_SampleTime_239Cycles5 );
	
	

	// ����ADC ��DMA ֧�֣�Ҫʵ��DMA ���ܣ������������DMA ͨ���Ȳ�����
	ADC_DMACmd(ADC1, ENABLE);
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE); 										//ʹ��ָ����ADC1
	/* Enable ADC1 reset calibaration register */
	ADC_ResetCalibration(ADC1); 								//��λָ����ADC1 ��У׼�Ĵ���
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1)); 				//��ȡADC1 ��λУ׼�Ĵ�����״̬,����״̬��ȴ�
	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1); 								//��ʼָ��ADC1 ��У׼״̬
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1)); 						//��ȡָ��ADC1 ��У׼����,����״̬��ȴ�
	
	
	//����DMA
	/* ADC1 DMA1 Channel Config */
	
	DMA_DeInit(DMA1_Channel1); 										//��DMA ��ͨ��1 �Ĵ�������Ϊȱʡֵ
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR; 		//DMA����ADC ����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value; 			//DMA �ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; 				//�ڴ���Ϊ���ݴ����Ŀ�ĵ�
	DMA_InitStructure.DMA_BufferSize = N*M; 						//DMA ͨ����DMA ����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 			//�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //���ݿ��Ϊ16 λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //���ݿ��Ϊ16 λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 					//������ѭ������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; 				//DMA ͨ�� x ӵ�и����ȼ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; 						//DMA ͨ��x û������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel1, &DMA_InitStructure); 						//����DMA_InitStruct ��ָ���Ĳ�����ʼ��DMA ��ͨ��

}

//ȥ�����������Сֵ 

uint16_t Average_off_maxandmin(uint16_t *Input,uint16_t Input_len)
{
	uint16_t i;
	uint16_t Max,Min,average;
	u32 sum=0;
	
	if(Input_len>2)
	{
		Min=Input[0];
		Max=Input[0];
		for(i=0;i<Input_len;i++)
		{
			if(Min>=Input[i])
			{
				Min=Input[i];
			}
			if(Max<=Input[i])
			{
				Max=Input[i];
			}
			sum+=Input[i];
		}
		sum-=(Max+Min);
		average=sum/(Input_len-2);
	}
	return average;	
}
/******************************************************************************
** ��������   void filter(void)
** ˵  ��:    
** ��ڲ����� 
** ���ڲ�����  
** ��  �أ�  
** ��  ע��   ��ƽ��ֵ����
******************************************************************************/

void filter(void)
{
	uint16_t BUF[N];
	uint16_t count;
	uint16_t i;
	for(i=0;i<M;i++)
	{
		for ( count=0;count<N;count++)
		{
			BUF[count] = AD_Value[count][i];									
		}		
	After_filter[i]=Average_off_maxandmin(BUF,50);
	}
}



void filter1(void)
{
	long int sum = 0;
	uint16_t count;
	uint16_t i;
	for(i=0;i<M;i++)
	{
		for ( count=0;count<N;count++)
		{
			sum += AD_Value[count][i];									
		}
	After_filter[i]=sum/N;
	sum=0;
	}
}
/******************************************************************************
** ��������   void Get_Value(uint16_t value[M])
** ˵  ��:    AD��ʼ������
** ��ڲ����� 
** ���ڲ�����  
** ��  �أ�  
** ��  ע��   
******************************************************************************/

void Get_Value(void)
{
		uint16_t i;
		/* Start ADC1 Software Conversion */
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		DMA_Cmd(DMA1_Channel1, ENABLE); 						//����DMA ͨ��
		
		filter();
		for(i=0;i<M;i++)
		{
			DATA_BUFF[i]= After_filter[i];
		}
}

/******************************************************************************/
uint16_t Loop_595(void)
{
	uint16_t tmp = 0;
	static uint16_t tmpreg = 0;

	for(tmp = 0; tmp < 8; tmp++)
	{
		
		SHCP_REG(1<<tmp);
		STCP_REG();
		
		/* ���������Ϊ�ߵ�ƽ ��Ϊ�ѽ������ */
		if(ID_BAT == 1)
		{
			tmpreg = 8 - tmp;	
			return tmpreg;
		}
		else
		{
			tmpreg = 0;
		}
	}
	return tmpreg;
}


uint16_t TYPE_BAT(uint16_t tmp)
{
	static uint16_t tmpreg = 0, num = 0;
	
	/* �������м�⵽ID_BAT=0 */
	if(ID_BAT == 0)
	{
		/* ��׼�����ٳ����ɣ�ά����ɣ�����,δ���״̬ */
		if(((CHAR_FLAG == 2) && (MAINTAIN_FLAG == 0)) || (MAINTAIN_FLAG == 4)
			||((MAINTAIN_FLAG == 2) && ((tim_rest_maintain_m_1 < TIM_MAINTAIN_REST) 
			|| (tim_rest_maintain_m_2 < TIM_MAINTAIN_REST)))
			|| ((CHAR_FLAG == 0) && (MAINTAIN_FLAG == 0)))
		{
			/* ���¼�� */
			tmpreg = Loop_595();
			return tmpreg;				
		}				
		/* ���ʱ����С��50��ŵ�ʱ����С��50 */
		else if(((I_CHAR < 50) && ((MAINTAIN_FLAG == 0) || (MAINTAIN_FLAG == 1) || (MAINTAIN_FLAG == 3))) 
			||((MAINTAIN_FLAG == 2) && (tim_rest_maintain_m_1 > TIM_MAINTAIN_REST) 
			&& (tim_rest_maintain_m_2 > TIM_MAINTAIN_REST) && (I_DISCHAR < 50)))
		{
			num++;
		}
		else
		{
			num = 0;
		}
		/* Ԥ����Ƿ���Բ��üӴ��жϣ� */
		if(PRE_FLAG == 1)
		{
			/* ����300�ε�����С50 */
			if(num >= 300)
			{
				num = 0;		
				/* ���¼�� */
				tmpreg = Loop_595();
				return tmpreg;			
			}
			else
			{
				/* ������һ�ε�ֵ */
				return tmp;
			}				
		}
		else
		{
			/* ����20�ε�����С50 */
			if(num >= 20)
			{
				num = 0;		
				/* ���¼�� */
				tmpreg = Loop_595();
				return tmpreg;			
			}
			else
			{
				/* ������һ�ε�ֵ */
				return tmp;
			}
		}
	}
	else
	{
		/* ������һ�ε�ֵ */
		return tmp;
	}
	
}




uint16_t DCINPUT_CURR(uint16_t init)
{
	uint16_t tmp = 0;
	static uint16_t re_tmp = 0;
	
	/* �޵���ʱ���г�ʼ�� */
	if(init == 0)
	{
		re_tmp = 0;
	}
	else
	{
		/* Ƿѹ�ض� */
		if(V_DCIN < VOL_IN_UNDER_OFF)
		{
			tmp = 0;
			return tmp;
		}
		/* ��������� */
		else if((V_DCIN >= VOL_IN_UNDER_OFF) && (V_DCIN < VOL_IN_UNDER_LP))
		{	
			re_tmp = 1;
			/* ���ֵΪ������� */
			tmp = IN_UNDER_FLG;
			return tmp;
		}
		/* ������� */
		else if((V_DCIN >= VOL_IN_UNDER_LP) && (V_DCIN < VOL_IN_OVER_OFF))
		{
			/* �ټ��һ��ֵ */
			if(re_tmp == 0)
			{
				tmp = IN_NORMEL_FLG;				
			}
			else if(re_tmp == 1)
			{
				/* ����һ����Ƿѹ��Χ�ڣ���˴�����18V������+1VΪ��������� */
				if(V_DCIN > VOL_IN_UNDER_LP + 103)
				{
					re_tmp = 0;
					tmp = IN_NORMEL_FLG;					
				}
				/* ������Ƿѹ */
				else
				{
					tmp = IN_UNDER_FLG;
				}
			}
			else
			{
				tmp = IN_NORMEL_FLG;
			}	
			return tmp;
		}
		/* ��ѹ�ض� */
		else
		{	
			tmp = IN_OFF_FLG;
			return tmp;			
		}
	}
	return tmp; 
}




