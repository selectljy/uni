#include "include.h"



									
uint16_t AD_Value[N][M]; 							//用来存放ADC 转换结果，也是DMA 的目标地址
uint16_t After_filter[M]; 							//用来存放求平均值之后的结果
uint16_t DATA_BUFF[M];								//RAM数据存储区--AD采样值

/******************************************************************************
** 函数名：   void GPIO_Configuration(void)
** 说  明:    ADC、DMA初始化函数
** 入口参数： 
** 出口参数：  
** 返  回：  
** 备  注：   
******************************************************************************/

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1,ENABLE ); //使能ADC1 通道时钟，各个管脚时钟
	/* Configure ADCCLK such as ADCCLK = PCLK2/6 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); 									//72M/6=12,ADC 最大时间不能超过14M
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); 					//使能DMA 传输
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 						
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 						
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	
	//ADC 初始化
	
	ADC_DeInit(ADC1); //将外设 ADC1 的全部寄存器重设为缺省值
	
	ADC_TempSensorVrefintCmd(ENABLE); 					//开启内部温度传感器
	
	/* ADC1 configuration ‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; 					//ADC 工作模式:ADC1 和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode =ENABLE; 						//模数转换工作在扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 					//模数转换工作在连续转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //外部触发转换关闭
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 				//ADC 数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = M;								 //顺序进行规则转换的ADC 通道的数目
	ADC_Init(ADC1, &ADC_InitStructure); 								//根据ADC_InitStruct 中指定的参数初始化外设ADCx 的寄存器
	/* ADC1 regular channel11 configuration */
	//设置指定ADC 的规则组通道，设置它们的转化顺序和采样时间
	//ADC1,ADC 通道x,规则采样顺序值为y,采样时间为239.5 周期
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 5, ADC_SampleTime_239Cycles5 );
	
	

	// 开启ADC 的DMA 支持（要实现DMA 功能，还需独立配置DMA 通道等参数）
	ADC_DMACmd(ADC1, ENABLE);
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE); 										//使能指定的ADC1
	/* Enable ADC1 reset calibaration register */
	ADC_ResetCalibration(ADC1); 								//复位指定的ADC1 的校准寄存器
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1)); 				//获取ADC1 复位校准寄存器的状态,设置状态则等待
	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1); 								//开始指定ADC1 的校准状态
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1)); 						//获取指定ADC1 的校准程序,设置状态则等待
	
	
	//配置DMA
	/* ADC1 DMA1 Channel Config */
	
	DMA_DeInit(DMA1_Channel1); 										//将DMA 的通道1 寄存器重设为缺省值
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR; 		//DMA外设ADC 基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value; 			//DMA 内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; 				//内存作为数据传输的目的地
	DMA_InitStructure.DMA_BufferSize = N*M; 						//DMA 通道的DMA 缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 			//内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //数据宽度为16 位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16 位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 					//工作在循环缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; 				//DMA 通道 x 拥有高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; 						//DMA 通道x 没有设置为内存到内存传输
	DMA_Init(DMA1_Channel1, &DMA_InitStructure); 						//根据DMA_InitStruct 中指定的参数初始化DMA 的通道

}

//去掉数组最大最小值 

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
** 函数名：   void filter(void)
** 说  明:    
** 入口参数： 
** 出口参数：  
** 返  回：  
** 备  注：   求平均值函数
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
** 函数名：   void Get_Value(uint16_t value[M])
** 说  明:    AD初始化函数
** 入口参数： 
** 出口参数：  
** 返  回：  
** 备  注：   
******************************************************************************/

void Get_Value(void)
{
		uint16_t i;
		/* Start ADC1 Software Conversion */
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		DMA_Cmd(DMA1_Channel1, ENABLE); 						//启动DMA 通道
		
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
		
		/* 电阻采样点为高电平 认为已接入电阻 */
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
	
	/* 充电过程中检测到ID_BAT=0 */
	if(ID_BAT == 0)
	{
		/* 标准、快速充电完成，维护完成，静置,未充电状态 */
		if(((CHAR_FLAG == 2) && (MAINTAIN_FLAG == 0)) || (MAINTAIN_FLAG == 4)
			||((MAINTAIN_FLAG == 2) && ((tim_rest_maintain_m_1 < TIM_MAINTAIN_REST) 
			|| (tim_rest_maintain_m_2 < TIM_MAINTAIN_REST)))
			|| ((CHAR_FLAG == 0) && (MAINTAIN_FLAG == 0)))
		{
			/* 重新检测 */
			tmpreg = Loop_595();
			return tmpreg;				
		}				
		/* 充电时电流小于50或放电时电流小于50 */
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
		/* 预充电是否可以不用加此判断？ */
		if(PRE_FLAG == 1)
		{
			/* 连续300次电流于小50 */
			if(num >= 300)
			{
				num = 0;		
				/* 重新检测 */
				tmpreg = Loop_595();
				return tmpreg;			
			}
			else
			{
				/* 返回上一次的值 */
				return tmp;
			}				
		}
		else
		{
			/* 连续20次电流于小50 */
			if(num >= 20)
			{
				num = 0;		
				/* 重新检测 */
				tmpreg = Loop_595();
				return tmpreg;			
			}
			else
			{
				/* 返回上一次的值 */
				return tmp;
			}
		}
	}
	else
	{
		/* 返回上一次的值 */
		return tmp;
	}
	
}




uint16_t DCINPUT_CURR(uint16_t init)
{
	uint16_t tmp = 0;
	static uint16_t re_tmp = 0;
	
	/* 无电阻时进行初始化 */
	if(init == 0)
	{
		re_tmp = 0;
	}
	else
	{
		/* 欠压关断 */
		if(V_DCIN < VOL_IN_UNDER_OFF)
		{
			tmp = 0;
			return tmp;
		}
		/* 降功率输出 */
		else if((V_DCIN >= VOL_IN_UNDER_OFF) && (V_DCIN < VOL_IN_UNDER_LP))
		{	
			re_tmp = 1;
			/* 这次值为正常输出 */
			tmp = IN_UNDER_FLG;
			return tmp;
		}
		/* 正常输出 */
		else if((V_DCIN >= VOL_IN_UNDER_LP) && (V_DCIN < VOL_IN_OVER_OFF))
		{
			/* 再检测一次值 */
			if(re_tmp == 0)
			{
				tmp = IN_NORMEL_FLG;				
			}
			else if(re_tmp == 1)
			{
				/* 若上一次在欠压范围内，则此次需在18V基础上+1V为正常输出点 */
				if(V_DCIN > VOL_IN_UNDER_LP + 103)
				{
					re_tmp = 0;
					tmp = IN_NORMEL_FLG;					
				}
				/* 否则还是欠压 */
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
		/* 过压关断 */
		else
		{	
			tmp = IN_OFF_FLG;
			return tmp;			
		}
	}
	return tmp; 
}




