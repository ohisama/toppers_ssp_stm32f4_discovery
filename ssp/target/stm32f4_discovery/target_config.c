/*
 *  TOPPERS/SSP Kernel
 *      Smallest Set Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2005-2007 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *
 *  Ported to STM32F4(-discovery) by Yasuhiro ISHII 2012
 *  Email : ishii.yasuhiro@gmail.com
 *
 *  �嵭����Ԥϡ��ʲ���(1)��(4)�ξ������������˸¤ꡤ�ܥ��եȥ���
 *  �����ܥ��եȥ���������Ѥ�����Τ�ޤࡥ�ʲ�Ʊ���ˤ���ѡ�ʣ������
 *  �ѡ������ۡʰʲ������ѤȸƤ֡ˤ��뤳�Ȥ�̵���ǵ������롥
 *  (1) �ܥ��եȥ������򥽡��������ɤη������Ѥ�����ˤϡ��嵭������
 *      ��ɽ�����������Ѿ�浪��Ӳ�����̵�ݾڵ��꤬�����Τޤޤη��ǥ���
 *      ����������˴ޤޤ�Ƥ��뤳�ȡ�
 *  (2) �ܥ��եȥ������򡤥饤�֥������ʤɡ�¾�Υ��եȥ�������ȯ�˻�
 *      �ѤǤ�����Ǻ����ۤ�����ˤϡ������ۤ�ȼ���ɥ�����ȡ�����
 *      �ԥޥ˥奢��ʤɡˤˡ��嵭�����ɽ�����������Ѿ�浪��Ӳ���
 *      ��̵�ݾڵ����Ǻܤ��뤳�ȡ�
 *  (3) �ܥ��եȥ������򡤵�����Ȥ߹���ʤɡ�¾�Υ��եȥ�������ȯ�˻�
 *      �ѤǤ��ʤ����Ǻ����ۤ�����ˤϡ����Τ����줫�ξ�����������
 *      �ȡ�
 *    (a) �����ۤ�ȼ���ɥ�����ȡ����Ѽԥޥ˥奢��ʤɡˤˡ��嵭����
 *        �ɽ�����������Ѿ�浪��Ӳ�����̵�ݾڵ����Ǻܤ��뤳�ȡ�
 *    (b) �����ۤη��֤��̤�������ˡ�ˤ�äơ�TOPPERS�ץ������Ȥ�
 *        ��𤹤뤳�ȡ�
 *  (4) �ܥ��եȥ����������Ѥˤ��ľ��Ū�ޤ��ϴ���Ū�������뤤���ʤ�»
 *      ������⡤�嵭����Ԥ����TOPPERS�ץ������Ȥ����դ��뤳�ȡ�
 *      �ޤ����ܥ��եȥ������Υ桼���ޤ��ϥ���ɥ桼������Τ����ʤ���
 *      ͳ�˴�Ť����ᤫ��⡤�嵭����Ԥ����TOPPERS�ץ������Ȥ�
 *      ���դ��뤳�ȡ�
 * 
 *  �ܥ��եȥ������ϡ�̵�ݾڤ��󶡤���Ƥ����ΤǤ��롥�嵭����Ԥ�
 *  ���TOPPERS�ץ������Ȥϡ��ܥ��եȥ������˴ؤ��ơ�����λ�����Ū
 *  ���Ф���Ŭ������ޤ�ơ������ʤ��ݾڤ�Ԥ�ʤ����ޤ����ܥ��եȥ���
 *  �������Ѥˤ��ľ��Ū�ޤ��ϴ���Ū�������������ʤ�»���˴ؤ��Ƥ⡤��
 *  ����Ǥ�����ʤ���
 * 
 */

/*
 * �������åȰ�¸�⥸�塼���CQ-STARM�ѡ�
 */
#include "kernel_impl.h"
#include <sil.h>
#include "cq_starm.h"
#include "target_serial.h"
#include "target_syssvc.h"



/* GPIO port mode register
   00b : Input
   01b : General purpose output mode
   10b : Alternate function mode
   11b : Analog mode
*/
Inline void set_port_mode(uint32_t reg,uint_t p,int_t v)
{
  sil_andw((void*)GPIO_MODER(reg),~(uint32_t)((0x03L << (p << 1))));
  sil_orw((void*)GPIO_MODER(reg),((uint32_t)(v & 0x03) << (p << 1)));
}

/* GPIO port output speed register
   00b :   2MHz
   01b :  25MHz
   10b :  50MHz
   11b : 100MHz
*/
Inline void set_port_speed(uint32_t reg,uint_t p,int_t v)
{
  sil_andw((void*)GPIO_OSPEEDR(reg),~(0x03L << (p << 1)));
  sil_orw((void*)GPIO_OSPEEDR(reg),((uint32_t)(v & 0x03) << (p << 1)));
}

/* GPIO port input data register
 */
Inline int_t get_port_data(uint32_t reg,uint_t p)
{
  return(!!(sil_rew_mem((void*)GPIO_IDR(reg)) & (1 << p)));
}

/* GPIO port output data register
*/
Inline void set_port_data(uint32_t reg,uint_t p,int_t v)
{
  sil_andw((void*)GPIO_ODR(reg),~(1L << p));
  sil_orw((void*)GPIO_ODR(reg),(v & 0x01) << p);
}


/* GPIO alternate function
 */
Inline void select_alternate_port_function(uint32_t reg,uint_t p,int_t v)
{
  if (p < 8){
    sil_andw((void*)GPIO_AFRL(reg),~(0x0FL << (p << 2)));
    sil_orw((void*)GPIO_AFRL(reg),((uint32_t)(v & 0x0f) << (p << 2)));
  } else {
    p -= 8;
    sil_andw((void*)GPIO_AFRH(reg),~(0x0FL << (p << 2)));
    sil_orw((void*)GPIO_AFRH(reg),((uint32_t)(v & 0x0f) << (p << 2)));
  }
}

/*
 * �������åȰ�¸�������������
 */
void target_initialize(void)
{

  sil_orw((void*)RCC_CR, CR_HSI_ON);
  
  // Reset CFGR Register : yishii
  sil_wrw_mem((void*)RCC_CFGR,0L);
  
  // Reset HSEON,CSSON and PLLON bits : yishii
  sil_andw((void*)RCC_CR,0xfef6ffff);
  
  sil_wrw_mem((void*)RCC_PLLCFGR,0x24003010);
  
  // reset HSEBYP bit
  sil_andw((void*)RCC_CR,0xfffbffff);
  
  sil_wrw_mem((void*)RCC_CIR,0L);
  
  ///////////////////////////////////////////////////////////////
  // SetSysClock
  
  // Enable HSE
  sil_orw((void*)RCC_CR, CR_HSE_ON);
  
  while ((sil_rew_mem((void*)RCC_CR) & CR_HSE_RDY) == 0);

  sil_orw((void*)RCC_APB1ENR,0x10000000); // APB1ENR.PWREN = set
  sil_orw((void*)PWR_CR,PWR_CR_VOS);

  // HCLK = SYSCLK / 1
  sil_orw((void*)RCC_CFGR,0);

  // PCLK2 = HCLK / 2
  sil_orw((void*)RCC_CFGR,0x00008000);

  // PCLK1 = HCLK / 4
  sil_orw((void*)RCC_CFGR,0x00001400);

  // Configure the main PLL

#define PLL_M      8
#define PLL_N    336
#define PLL_P      2
#define RCC_PLLCFGR_PLLSRC_HSE              ((uint32_t)0x00400000)
#define PLL_Q      7

  sil_wrw_mem((void*)RCC_PLLCFGR,
	      PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) |
	      (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24));

  sil_orw((void*)RCC_CR,CR_PLL_ON);

  // wait for PLL Ready
  while ((sil_rew_mem((void*)RCC_CR) & CR_PLL_RDY) == 0);

  /* Configure Flash prefetch, Instruction cache, Data cache and wait state */
  sil_wrw_mem((void*)FLASH_ACR,FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_5WS);

  // Select the main PLL as system clock source
  sil_andw((void*)RCC_CFGR,(uint32_t)~CFGR_SW_MASK);
  sil_orw((void*)RCC_CFGR,CFGR_SW_PLL);


	// AHB1 GPIO Clock enable for STM32F4
	sil_orw((void*)RCC_AHB1ENR,0x000001FF);

	//  all enable(tmp)
	sil_orw((void*)RCC_APB1ENR,0x36fec9ff);

	// APB2 USART1 Clock enable for STM32F4
	sil_orw((void*)RCC_APB2ENR,0x00000010);

	// all enable(tmp)
	sil_orw((void*)RCC_APB2ENR,0x00075f33);


	/*
	 *  �ץ��å���¸���ν����
	 */
	prc_initialize();
	/*
	 *  I/O�ݡ��Ȥν����
	 */



	/* USART1 @ USART1_REMAP = 0
	   TX : PA9
	   RX : PA10
	*/

	/* USART1(RX) */
	//set_port_mode(GPIOA_BASE,10,2);
	set_port_mode(GPIOB_BASE,7,2); // test!!


	/* USART1(TX) */
	//set_port_speed(GPIOA_BASE,9,MODE_OUTPUT_50MHZ);
	//set_port_mode(GPIOA_BASE,9,2);

	set_port_speed(GPIOB_BASE,6,MODE_OUTPUT_50MHZ);
	set_port_mode(GPIOB_BASE,6,2);

	/* select alternate pin function */
	//select_alternate_port_function(GPIOA_BASE,9,7);
	//select_alternate_port_function(GPIOA_BASE,10,7);
	select_alternate_port_function(GPIOB_BASE,7,7);
	select_alternate_port_function(GPIOB_BASE,6,7);
	/* LED�ݡ��� */

	// for STM32F4-discovery

	set_port_speed(GPIOD_BASE,12,MODE_OUTPUT_50MHZ);
	set_port_speed(GPIOD_BASE,13,MODE_OUTPUT_50MHZ);
	set_port_speed(GPIOD_BASE,14,MODE_OUTPUT_50MHZ);
	set_port_speed(GPIOD_BASE,15,MODE_OUTPUT_50MHZ);

	// LED(ORANGE)
	set_port_mode(GPIOD_BASE,13,1);
	set_port_data(GPIOD_BASE,13,1); // for debug
	// LED(GREEN)
	set_port_mode(GPIOD_BASE,12,1);
	set_port_data(GPIOD_BASE,12,1); // for debug
	// LED(RED)
	set_port_mode(GPIOD_BASE,14,1);
	set_port_data(GPIOD_BASE,14,1); // for debug
	// LED(BLUE)
	set_port_mode(GPIOD_BASE,15,1);
	set_port_data(GPIOD_BASE,15,1); // for debug

	/*
	 *  �Хʡ������ѤΥ��ꥢ������
	 */
	target_low_output_init(SIO_PORTID);

}

/*
 * �������åȰ�¸�� ��λ����
 */
void target_exit(void)
{
	/* �ץ��å���¸���ν�λ���� */
	prc_terminate();
	
	while(true)
		;
}

/*
 * �����ƥ�������٥���ϤΤ����ʸ������
 */
void target_fput_log(char_t c)
{
	if (c == '\n') {
		sio_pol_snd_chr('\r', SIO_PORTID);
	}
	sio_pol_snd_chr(c, SIO_PORTID);
}
